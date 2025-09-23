#pragma once

#include "Simulator.hpp"
#include "CliffordState.h"

#define RC_DEFAULT_GATE_WIDTH 2

#define RC_DEFAULT_CLIFFORD_SIMULATOR "chp"

#define RC_DEFAULT_PBC true

#define RC_BRICKWORK 0
#define RC_RANDOM_LOCAL 1
#define RC_RANDOM_NONLOCAL 2
#define RC_POWERLAW 3


inline static void rc_timestep(std::shared_ptr<CliffordState> state, uint32_t gate_width, bool offset_layer, bool periodic_bc = true) {
	uint32_t system_size = state->num_qubits;
	uint32_t num_gates = system_size / gate_width;

	std::vector<uint32_t> qubits(gate_width);
	std::iota(qubits.begin(), qubits.end(), 0);

	for (uint32_t j = 0; j < num_gates; j++) {
		uint32_t offset = offset_layer ? gate_width*j : gate_width*j + gate_width/2;

		bool periodic = false;
		std::vector<uint32_t> offset_qubits(qubits);
		std::transform(offset_qubits.begin(), offset_qubits.end(), offset_qubits.begin(), 
						[system_size, offset, &periodic](uint32_t x) { 
							uint32_t q = x + offset;
							if (q % system_size != q) {
								periodic = true;
							}
							return q % system_size; 
						});
		
		if (!(!periodic_bc && periodic)) {
			state->random_clifford(offset_qubits);
		}
	}
}

static inline double rc_power_law(double x0, double x1, double n, double r) {
	return std::pow(((std::pow(x1, n + 1.0) - std::pow(x0, n + 1.0))*r + std::pow(x0, n + 1.0)), 1.0/(n + 1.0));
}

static Color RC_COLOR1 = {246.0/255, 77.0/255, 77.0/255, 1.0};
static Color RC_COLOR2 = {77.0/255, 105.0/255, 246.0/255, 1.0};
static Color RC_COLOR3 = {189.0/255, 74.0/255, 218.0/255, 1.0};

class RandomCliffordSimulator : public Simulator {
	private:
		int seed;

		uint32_t system_size;
		double mzr_prob;
		uint32_t gate_width;
		uint32_t timestep_type;
		double alpha;

		std::string simulator_type;
		
		bool offset;
		bool pbc;

    uint32_t randpl() {
      return rc_power_law(1.0, system_size/2.0, -alpha, randf()); 
    }

    void timestep_random_local() {
      uint32_t q1 = rand() % system_size;
      uint32_t q2 = (q1 + 1) % system_size;

      std::vector<uint32_t> qbits{q1, q2};;
      state->random_clifford(qbits);

      if (randf() < mzr_prob) {
        mzr(randi() % system_size);
      }
    }

    void timestep_random_nonlocal() {
      uint32_t q1 = randi() % system_size;
      uint32_t q2 = randi() % system_size;
      while (q2 == q1) {
        q2 = randi() % system_size;
      }

      std::vector<uint32_t> qbits{q1, q2};;
      state->random_clifford(qbits);

      if (randf() < mzr_prob) {
        mzr(randi() % system_size);
      }
    }

    void timestep_powerlaw() {
      uint32_t q1 = randi() % system_size;
      uint32_t dq = randpl();
      uint32_t q2 = (randi() % 2) ? mod(q1 + dq, system_size) : mod(q1 - dq, system_size);

      std::vector<uint32_t> qbits{q1, q2};;
      state->random_clifford(qbits);

      if (randf() < mzr_prob) {
        mzr(randi() % system_size);
      }
    }

    void timestep_brickwork(uint32_t num_steps) {
      if (system_size % gate_width != 0) {
        throw std::invalid_argument("Invalid gate width. Must divide system size.");
      } if (gate_width % 2 != 0) {
        throw std::invalid_argument("Gate width must be even.");
      }

      for (uint32_t i = 0; i < num_steps; i++) {
        rc_timestep(state, gate_width, offset, pbc);

        // Apply measurements
        for (uint32_t j = 0; j < system_size; j++) {
          if (randf() < mzr_prob) {
            mzr(j);
          }
        }

        offset = !offset;
      }
    }

    void mzr(uint32_t q) {
      state->mzr(q);
    }

	public:
		std::shared_ptr<QuantumCHPState> state;

		RandomCliffordSimulator(Params& params) {
      system_size = get<int>(params, "system_size");

      mzr_prob = get<double>(params, "mzr_prob");
      gate_width = get<int>(params, "gate_width", RC_DEFAULT_GATE_WIDTH);

      timestep_type = get<int>(params, "timestep_type", RC_BRICKWORK);
      if (timestep_type == RC_POWERLAW) {
        alpha = get<double>(params, "alpha");
      }

      simulator_type = get<std::string>(params, "simulator_type", RC_DEFAULT_CLIFFORD_SIMULATOR);

      offset = false;
      pbc = get<int>(params, "pbc", RC_DEFAULT_PBC);

      state = std::make_shared<QuantumCHPState>(system_size);
    }

    virtual void timesteps(uint32_t num_steps) override {
      if (timestep_type == RC_BRICKWORK) {
        timestep_brickwork(num_steps);
      } else if (timestep_type == RC_RANDOM_LOCAL) {
        timestep_random_local();
      } else if (timestep_type == RC_RANDOM_NONLOCAL) {
        timestep_random_nonlocal();
      } else if (timestep_type == RC_POWERLAW) {
        timestep_powerlaw();
      }

      state->tableau->rref();
    }
};

