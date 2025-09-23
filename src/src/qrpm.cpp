#include "webgl.hpp"
#include "QRPM/SandpileCliffordSimulator.h"

extern "C" {
  EMSCRIPTEN_KEEPALIVE
  void slider_callback(int i) {

  }

  EMSCRIPTEN_KEEPALIVE
  void button_callback() {
    SandpileCliffordSimulator* simulator = static_cast<SandpileCliffordSimulator*>(program->simulator.get());
    simulator->advance_feedback_strategy();
  }

  EMSCRIPTEN_KEEPALIVE
  void radial_callback(double pu, double pm) {
    SandpileCliffordSimulator* simulator = static_cast<SandpileCliffordSimulator*>(program->simulator.get());
    simulator->set_strengths(pu, pm);
  }
}

std::shared_ptr<Simulator> prepare_model() {
  Params params;
  params.emplace("system_size", 64);
  params.emplace("unitary_prob", 0.5);
  params.emplace("mzr_prob", 0.5);
  params.emplace("unitary_qubits", 4);
  params.emplace("mzr_mode", 1);

  return std::make_shared<SandpileCliffordSimulator>(params);
}

int main() {
  glProgram program = init_gl();
  program.bind_simulator(prepare_model());
  std::cout << "Program created\n";
  int fps = 60;
  emscripten_set_main_loop_arg(draw_frame, &program, 0, EM_TRUE);
  return 0;
}

