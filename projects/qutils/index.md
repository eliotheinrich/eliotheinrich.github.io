---
layout: home 
title: qutils
author_profile: true
permalink: /projects/qutils/
---

https://github.com/eliotheinrich/qutils

`qutils` is, in essence, the corpus of code that I wrote and use for the simulations in my quantum research. As a result,
many of the features are built around the specific needs of my research. Nonetheless, I think these tools may be useful for others,
so I have done my best to keep the code organized, modular, and simple to use for non-programmers. `qutils` is natively written in C++,
but the package provides robust python bindings which serve as the primary point of contact.

The primary goal of `qutils` is to provide a modular and efficient representation of various types of quantum states subject to trajectory evolution. 
This is particularly useful for characterizing non-linear quantities of quantum states such as entanglement when states are subjected to mid-circuit
measurements and classical communication.

Currently, statevectors, density matrices, stabilizer states, matrix product states, and fermionic gaussian states all supported.
Any of these states can be evolved by arbitrary quantum circuits, which can contains various instructions. These instructions include
unitary evolution, projective measurements, weak measurements, instructions on corresponding classical data, and hybrid instructions (i.e.
classically conditioned unitary operations). These instructions can also be parametrized, allowing for variational quantum algorithms
to be executed in `qutils`.

Below are a few snippets demonstrating experiments on systems with many qubits that can be performed with `qutils` on a laptop. 

First, we can create a GHZ state. Let's do it with both a statevector and a stabilizer state.
```
$ cat ghz.py
from qutils import Statevector, QuantumCHPState, QuantumCircuit

num_qubits = 20

circuit = QuantumCircuit(num_qubits)
circuit.h(0)
for q in range(1, num_qubits):
    circuit.cx(0, q)

psi = Statevector(num_qubits)
psi.evolve(circuit)

chp = QuantumCHPState(num_qubits)
chp.evolve(circuit)

# Print the statevector
print(psi)

# Print the von Neumann entanglement entropy of the first num_qubits/2 qubits
A = list(range(num_qubits//2))
renyi_index = 1
print(f"Statevector has entanglement      {psi.entanglement(A, renyi_index)}")
print(f"Stabilizer state has entanglement {chp.entanglement(A, renyi_index)}")

$ python ghz.py
0.7071067811865476|00000000000000000000> + 0.7071067811865476|11111111111111111111>
Statevector has entanglement      0.9999999999999999
Stabilizer state has entanglement 1.0
```

Matrix product states (MPS) allow for simulation of quantum systems with many qubits, so long as the entanglement is small. In particular, 
an MPS simulation scales roughly as $O(N\chi^3)$, where $N$ is the number of qubits and $\chi$ is the bond dimension. The entanglement scales
roughly as $O(\log \chi)$. Therefore, an MPS is an 'efficient' representation when the entanglement is (sub)logarithmic. This is typically the
point in entanglement area-law phases and quantum critical points. 

```
$ cat mps.py
from qutils import MatrixProductState, QuantumCircuit

num_qubits = 100
bond_dimension = 16

circuit = QuantumCircuit(num_qubits)
for q in range(num_qubits-1):
    circuit.h(q)
    circuit.cx(q, q+1)

mps = MatrixProductState(num_qubits, bond_dimension)

# Print the von Neumann entanglement entropy of the first num_qubits/2 qubits
A = list(range(num_qubits//2))
renyi_index = 1
print(f"MPS has entanglement {mps.entanglement(A, renyi_index)}")

$ python mps.py
MPS has entanglement 0.9999999999999999
```

Another quantum state of interest is fermionic gaussian states, which can efficiently represent quantums states generated
from an initial state of populated fermions evolved under matchgate unitaries and projective measurements. Fermionic operators
are mapped onto qubits via the Jordan-Wigner transform in the usual way:
$$c_i = K_j (X_j + iY_j), \quad c_j^\dagger = K_j (X_j - i Y_j), \quad K_j = \prod\limits_{k < j} Z_k$$

Matchgates are gates which can be represented as unitary evolution as free fermions

$$U_{MG}(t) = e^{i t H}, \quad H = \sum\limits_{i,j} a_{ij} c_i^\dagger c_j + \sum\limits_{i, j} b_{ij} c_i c_j + h.c.$$

where $X_j$, $Y_j$, and $Z_j$ are the corresponding Pauli operators.

Matchgates have a special representation in `qutils`. When a matchgate is applied to a fermionic gaussian state, the operator is
efficiently applied to the internal representation of the state. Otherwise, the gate is converted via the Jordan-Wigner transform
into a unitary acting on qubits. This can be useful for simulating circuits of small size dominated by free fermion dynamics with 
doped interaction terms which cannot be applied to gaussian states.

```
$ cat fgs.py
from qutils import GaussianState, QuantumCircuit

num_qubits = 100
bond_dimension = 16

circuit = QuantumCircuit(num_qubits)
for q in range(num_qubits-1):
    circuit.h(q)
    circuit.cx(q, q+1)

mps = MatrixProductState(num_qubits, bond_dimension)

# Print the von Neumann entanglement entropy of the first num_qubits/2 qubits
A = list(range(num_qubits//2))
renyi_index = 1
print(f"MPS has entanglement {mps.entanglement(A, renyi_index)}")

$ python mps.py
MPS has entanglement 0.9999999999999999
```

`qutils` is a work in progress, so there may be some situations that I have not accounted for. You can see the tests in 
`qutils/src/Tests`, and if you run into any issues feel free to open an issue on the repository!
