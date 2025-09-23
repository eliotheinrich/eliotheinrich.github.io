#include "SandpileCliffordSimulator.h"
#include "RandomCliffordSimulator.hpp"
#include "QuadCollection.hpp"

#define DEFAULT_BOUNDARY_CONDITIONS "pbc"
#define DEFAULT_FEEDBACK_MODE 22
#define DEFAULT_UNITARY_QUBITS false
#define DEFAULT_MZR_MODE 1

#define DEFAULT_SAMPLE_AVALANCHES false

#define SUBSTRATE 0
#define PYRAMID 1 
#define RANDOM 2

SandpileCliffordSimulator::SandpileCliffordSimulator(Params& params) {
  rng = std::minstd_rand(randi());
  system_size = get<int>(params, "system_size");

  mzr_prob = get<double>(params, "mzr_prob");
  unitary_prob = get<double>(params, "unitary_prob");
  params.emplace("u", unitary_prob/mzr_prob);

  boundary_condition = get<std::string>(params, "boundary_conditions", DEFAULT_BOUNDARY_CONDITIONS);
  feedback_mode = get<int>(params, "feedback_mode", DEFAULT_FEEDBACK_MODE);
  unitary_qubits = get<int>(params, "unitary_qubits", DEFAULT_UNITARY_QUBITS);
  mzr_mode = get<int>(params, "mzr_mode", DEFAULT_MZR_MODE);

  initial_state = get<int>(params, "initial_state", SUBSTRATE);
  scrambling_steps = get<int>(params, "scrambling_steps", system_size);

  // ------------------ TETRONIMOS -------------------
  // |  (1)  |  (2)  |  (3)  |  (4)  |  (5)  |  (6)  |
  // |       |       |       |       |       |     o |
  // |	   |   o   | o   o |     o |   o o |   o o |
  // | o o o | o o o | o o o | o o o | o o o | o o o |
  set_feedback_strategy(feedback_mode);

  state = std::make_shared<QuantumCHPState>(system_size);

  if (initial_state == SUBSTRATE) {
    // Do nothing
  } else if (initial_state == PYRAMID) {
    bool offset = 0;

    for (uint32_t k = 0; k < scrambling_steps; k++) {
      rc_timestep(state, 2, offset, true);
      offset = !offset;
    }
  } else if (initial_state == RANDOM) {
    std::vector<uint32_t> all_sites(system_size);
    std::iota(all_sites.begin(), all_sites.end(), 0);
    state->random_clifford(all_sites);
  }
}

void SandpileCliffordSimulator::set_feedback_strategy(int feedback_mode) {
  this->feedback_mode = feedback_mode;
  std::cout << std::format("Called advance feedback_strategy({})\n", feedback_mode);
  if (feedback_mode == 0)       feedback_strategy = std::vector<uint32_t>{1};
  else if (feedback_mode == 1)  feedback_strategy = std::vector<uint32_t>{1, 2};
  else if (feedback_mode == 2)  feedback_strategy = std::vector<uint32_t>{1, 3};
  else if (feedback_mode == 3)  feedback_strategy = std::vector<uint32_t>{1, 4};
  else if (feedback_mode == 4)  feedback_strategy = std::vector<uint32_t>{1, 5};
  else if (feedback_mode == 5)  feedback_strategy = std::vector<uint32_t>{1, 6};
  else if (feedback_mode == 6)  feedback_strategy = std::vector<uint32_t>{1, 2, 3};
  else if (feedback_mode == 7)  feedback_strategy = std::vector<uint32_t>{1, 2, 4};
  else if (feedback_mode == 8)  feedback_strategy = std::vector<uint32_t>{1, 2, 5};
  else if (feedback_mode == 9)  feedback_strategy = std::vector<uint32_t>{1, 2, 6};
  else if (feedback_mode == 10) feedback_strategy = std::vector<uint32_t>{1, 3, 4};
  else if (feedback_mode == 11) feedback_strategy = std::vector<uint32_t>{1, 3, 5};
  else if (feedback_mode == 12) feedback_strategy = std::vector<uint32_t>{1, 3, 6};
  else if (feedback_mode == 13) feedback_strategy = std::vector<uint32_t>{1, 4, 5};
  else if (feedback_mode == 14) feedback_strategy = std::vector<uint32_t>{1, 4, 6};
  else if (feedback_mode == 15) feedback_strategy = std::vector<uint32_t>{1, 5, 6};
  else if (feedback_mode == 16) feedback_strategy = std::vector<uint32_t>{1, 2, 3, 4};
  else if (feedback_mode == 17) feedback_strategy = std::vector<uint32_t>{1, 2, 3, 5};
  else if (feedback_mode == 18) feedback_strategy = std::vector<uint32_t>{1, 2, 3, 6};
  else if (feedback_mode == 19) feedback_strategy = std::vector<uint32_t>{1, 2, 4, 5};
  else if (feedback_mode == 20) feedback_strategy = std::vector<uint32_t>{1, 2, 4, 6};
  else if (feedback_mode == 21) feedback_strategy = std::vector<uint32_t>{1, 2, 5, 6};
  else if (feedback_mode == 22) feedback_strategy = std::vector<uint32_t>{1, 3, 4, 5};
  else if (feedback_mode == 23) feedback_strategy = std::vector<uint32_t>{1, 3, 4, 6};
  else if (feedback_mode == 24) feedback_strategy = std::vector<uint32_t>{1, 3, 5, 6};
  else if (feedback_mode == 25) feedback_strategy = std::vector<uint32_t>{1, 4, 5, 6};
  else if (feedback_mode == 26) feedback_strategy = std::vector<uint32_t>{1, 2, 3, 4, 5};
  else if (feedback_mode == 27) feedback_strategy = std::vector<uint32_t>{1, 2, 3, 4, 6};
  else if (feedback_mode == 28) feedback_strategy = std::vector<uint32_t>{1, 2, 3, 5, 6};
  else if (feedback_mode == 29) feedback_strategy = std::vector<uint32_t>{1, 2, 4, 5, 6};
  else if (feedback_mode == 30) feedback_strategy = std::vector<uint32_t>{1, 3, 4, 5, 6};
  else if (feedback_mode == 31) feedback_strategy = std::vector<uint32_t>{1, 2, 3, 4, 5, 6};
}

void SandpileCliffordSimulator::advance_feedback_strategy() {
  set_feedback_strategy((feedback_mode + 1) % 32);
}

void SandpileCliffordSimulator::set_strengths(double pu, double pm) {
  unitary_prob = pu;
  mzr_prob = pm;
}

void SandpileCliffordSimulator::mzr(uint32_t i) {
  if (randf() < mzr_prob) {
    // Do measurement
    if (mzr_mode == 0) {
      state->mzr(i);
    } else if (mzr_mode == 1) {
      state->mzr(i);
      state->mzr(i+1);
    } else if (mzr_mode == 2) {
      if (randf() < 0.5) {
        state->mzr(i);
      } else {
        state->mzr(i+1);
      }
    }

  }
}

void SandpileCliffordSimulator::unitary(uint32_t i) {
  if (randf() < unitary_prob) {

    std::vector<uint32_t> qubits;
    if (unitary_qubits == 2) {
      qubits = std::vector<uint32_t>{i, i+1};
    } else if (unitary_qubits == 3) {
      qubits = std::vector<uint32_t>{i-1, i, i+1};
    } else if (unitary_qubits == 4) {
      qubits = std::vector<uint32_t>{i-1, i, i+1, i+2};
    }

    state->random_clifford(qubits);
  }
}

void SandpileCliffordSimulator::timesteps(uint32_t num_steps) {
  for (uint32_t k = 0; k < num_steps; k++) {
    timestep();
  }
}

void SandpileCliffordSimulator::left_boundary() {
  if (boundary_condition == "pbc") {
    std::vector<uint32_t> qubits{0, 1};
    state->random_clifford(qubits);
  } else if (boundary_condition == "obc1") {

  } else if (boundary_condition == "obc2") {
    std::vector<uint32_t> qubits{0, 1};
    state->random_clifford(qubits);
  } else { 
    std::string error_message = "Invalid boundary condition: " + boundary_condition;
    throw std::invalid_argument(error_message);
  }
}

void SandpileCliffordSimulator::right_boundary() {
  if (boundary_condition == "pbc") {
    std::vector<uint32_t> qubits{system_size-1, 0};
    state->random_clifford(qubits);
  } else if (boundary_condition == "obc1") {

  } else if (boundary_condition == "obc2") {
    std::vector<uint32_t> qubits{system_size-2, system_size-1};
    state->random_clifford(qubits);
  }
}

uint32_t SandpileCliffordSimulator::get_shape(uint32_t s0, uint32_t s1, uint32_t s2) const {
  int ds1 = s0 - s1;
  int ds2 = s2 - s1;

  if      ((ds1 == 0)  && (ds2 == 0))   return 1; // 0 0 0 (a)
  else if ((ds1 == -1) && (ds2 == -1))  return 2; // 0 1 0 (b)
  else if ((ds1 == 1)  && (ds2 == 1))   return 3; // 1 0 1 (c)
  else if ((ds1 == 0)  && (ds2 == 1))   return 4; // 0 0 1 (d)
  else if ((ds1 == 1)  && (ds2 == 0))   return 4; // 1 0 0
  else if ((ds1 == 0)  && (ds2 == -1))  return 5; // 1 1 0 (e)
  else if ((ds1 == -1) && (ds2 == 0))   return 5; // 0 1 1
  else if ((ds1 == -1) && (ds2 == 1))   return 6; // 0 1 2 (f)
  else if ((ds1 == 1)  && (ds2 == -1))  return 6; // 2 1 0
  else { 
    throw std::invalid_argument("Something has gone wrong with the entropy substrate."); 
  }
}

void SandpileCliffordSimulator::feedback(uint32_t q) {
  uint32_t q0;
  uint32_t q2;
  if (boundary_condition == "pbc") {
    q0 = mod(q - 1, system_size);
    q2 = mod(q + 1, system_size);
  } else {
    q0 = q - 1;
    q2 = q + 1;
  }

  int s0 = state->cum_entanglement<int>(q0);
  int s1 = state->cum_entanglement<int>(q);
  int s2 = state->cum_entanglement<int>(q2);

  uint32_t shape = get_shape(s0, s1, s2);

  if (std::count(feedback_strategy.begin(), feedback_strategy.end(), shape)) {
    unitary(q);
  } else {
    mzr(q);
  }
}

void SandpileCliffordSimulator::timestep() {
  left_boundary();

  std::uniform_int_distribution<> dis(1, system_size - 3);

  for (uint32_t i = 0; i < system_size; i++) {
    uint32_t q = dis(rng);
    feedback(q);
  }

  right_boundary();
}


void SandpileCliffordSimulator::draw() const {
  std::vector<double> s = state->get_entanglement<double>();

  std::vector<float> vertices;
  std::vector<unsigned int> indices;
  double width = 1.0/system_size;
  for (size_t i = 0; i < system_size; i++) {
    double x = 2.0*(double(i) / system_size - 0.5);
    double height = 2.0*s[i]/system_size;
    auto [vert, idx] = make_quad(x, -0.5, 2*width, height, vertices.size()/3);
    vertices.insert(vertices.begin(), vert.begin(), vert.end());
    indices.insert(indices.begin(), idx.begin(), idx.end());
  }

  QuadCollection quads;
  const Color substrate_color = {64/256.0, 224/256., 208/256., 1.0};
  quads.add_vertices(vertices, indices, substrate_color);
  quads.draw();
}

static std::string to_upper(const std::string& str) {
  std::string s = str;
  std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return std::toupper(c); });
  return s;
}

void SandpileCliffordSimulator::key_callback(const std::string& key) {
  std::string upper_key = to_upper(key);
  if (upper_key == "ARROWLEFT") {
    mzr_prob = std::min(1.0, mzr_prob + 0.01); 
  } else if (upper_key == "ARROWRIGHT") {
    mzr_prob = std::max(0.0, mzr_prob - 0.01);
  } else if (upper_key == "ARROWUP") {
    unitary_prob = std::min(1.0, unitary_prob + 0.01); 
  } else if (upper_key == "ARROWDOWN") {
    unitary_prob = std::max(0.0, unitary_prob - 0.01);
  } else if (upper_key == " ") {
    advance_feedback_strategy();
  }

  std::cout << std::format("Registed keypress. Probs = {}, {}\n", unitary_prob, mzr_prob);
}
