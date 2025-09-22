#include "webgl.hpp"
#include "Ising/SquareIsingModel.h"
#include "Spin2d/SquareXYModel.h"

extern "C" {
  EMSCRIPTEN_KEEPALIVE
  void slider_callback(double T) {
    std::cout << std::format("Called slider_callback({})\n", T);
    MonteCarloSimulator* simulator = static_cast<MonteCarloSimulator*>(program->simulator.get());
    simulator->temperature = T;
  }

  EMSCRIPTEN_KEEPALIVE
  void button_callback() {
    SquareXYModel* simulator = static_cast<SquareXYModel*>(program->simulator.get());
    simulator->cluster_update = !simulator->cluster_update;
  }

  EMSCRIPTEN_KEEPALIVE
  void radial_callback(double b, double phi) {
    SquareXYModel* simulator = static_cast<SquareXYModel*>(program->simulator.get());
    simulator->set_field(b, phi);
  }
}

std::shared_ptr<Simulator> prepare_model() {
  Params params;
  params.emplace("system_size", 256);
  params.emplace("J", 1.0);
  params.emplace("B", 0.0);
  params.emplace("Bp", 0.0);
  params.emplace("temperature", 1.0);
  params.emplace("cluster_update", 0);

  return std::make_shared<SquareXYModel>(params, 1);
}

int main() {
  glProgram program = init_gl();
  program.bind_simulator(prepare_model());
  int fps = 60;
  emscripten_set_main_loop_arg(draw_frame, &program, 0, EM_TRUE);
  return 0;
}

