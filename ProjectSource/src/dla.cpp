#include "webgl.hpp"
#include "DLA/DiffusionController.hpp"

extern "C" {
  EMSCRIPTEN_KEEPALIVE
  void slider_callback(int i) {

  }

  EMSCRIPTEN_KEEPALIVE
  void button_callback() {

  }

  EMSCRIPTEN_KEEPALIVE
  void radial_callback(double b, double phi) {

  }
}

std::shared_ptr<Simulator> prepare_model() {
  Params params;
  params.emplace("n", 256);
  params.emplace("m", 256);
  params.emplace("target_particle_density", 0.1);

  return std::make_shared<DiffusionController>(params);
}

int main() {
  glProgram program = init_gl();
  program.bind_simulator(prepare_model());
  std::cout << "Program created\n";
  int fps = 60;
  emscripten_set_main_loop_arg(draw_frame, &program, 0, EM_TRUE);
  return 0;
}

