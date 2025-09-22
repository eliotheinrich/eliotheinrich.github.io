#include <memory>
#include <set>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <format>
#include <map> 

#include "Simulator.hpp"
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include <GLES3/gl3.h>

EM_BOOL key_callback(int eventType, const EmscriptenKeyboardEvent *e, void *userData);



class glProgram {
  public:
    std::set<std::string> keys_pressed;
    std::map<std::string, std::chrono::steady_clock::time_point> key_timer;

    std::shared_ptr<Simulator> simulator;
    int width;
    int height;

    double time;

    int get_width() const {
      return width;
    }

    int get_height() const {
      return height;
    } 

    void process_input() {
      for (const auto& key : keys_pressed) {
        press(key);
      }
    }

    void press(const std::string& key) {
      if (key_timer.contains(key)) {
        auto now = std::chrono::steady_clock::now();
        float delta = (now - key_timer.at(key)).count()/1e9;
        if (delta < 0.1) {
          return;
        }

        std::cout << std::format("It's been {} since key {} was pressed.\n", delta, key);
      }

      key_timer[key] = std::chrono::steady_clock::now();
      execute_press(key);
    }

    void execute_press(const std::string& key) {
      simulator->key_callback(key);
    }

    glProgram(int width, int height) : width(width), height(height) {
      emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, this, EM_TRUE, key_callback);
      emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, this, EM_TRUE, key_callback);
    }

    void bind_simulator(std::shared_ptr<Simulator> simulator) {
      this->simulator = simulator;
    }

    void advance_frame(size_t num_steps) {
      simulator->timesteps(1);
    }

    void draw() {
      simulator->draw();
      process_input();
    }
};

EM_BOOL key_callback(int eventType, const EmscriptenKeyboardEvent *e, void *userData) {
  glProgram* program = static_cast<glProgram*>(userData);

  if (eventType == EMSCRIPTEN_EVENT_KEYDOWN) {
    program->keys_pressed.insert(e->key);
  } else if (eventType == EMSCRIPTEN_EVENT_KEYUP) {
    program->keys_pressed.erase(e->key);
  }

  return EM_TRUE;
}

extern "C" {
  glProgram* program = nullptr;

  EMSCRIPTEN_KEEPALIVE
  void set_program_pointer(glProgram* ptr) {
    program = ptr;
  }
}

glProgram init_gl() {
  // Initialize context and make current
  EmscriptenWebGLContextAttributes attr;
  emscripten_webgl_init_context_attributes(&attr);
  attr.majorVersion = 2; // Request WebGL2 for float textures
  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx = emscripten_webgl_create_context("#canvas", &attr);
  emscripten_webgl_enable_extension(ctx, "OES_texture_float");
  emscripten_webgl_enable_extension(ctx, "EXT_color_buffer_float");

  int w, h;
  emscripten_get_screen_size(&w, &h);
  EMSCRIPTEN_RESULT r = emscripten_set_canvas_element_size("#canvas", w, h);
  emscripten_webgl_make_context_current(ctx);

  glProgram program(w, h);
  set_program_pointer(&program);

  return program;
}

int frame = 0;

void draw_frame(void* arg) {
  glProgram* program = static_cast<glProgram*>(arg);
  glViewport(0, 0, program->get_width(), program->get_height());
  glClear(GL_COLOR_BUFFER_BIT);

  program->advance_frame(1);
  program->draw();
}
