#pragma once

#include <variant>
#include <map>
#include <format>
#include <vector>

#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include <GLES3/gl3.h>

using Parameter = std::variant<std::string, int, double>;
using Params = std::map<std::string, Parameter>;

template <class T>
T get(Params &params, const std::string& key, T defaultv) {
  if (params.count(key)) {
    return std::get<T>(params[key]);
  }

  params[key] = Parameter{defaultv};
  return defaultv;
}

template <class T>
T get(const Params &params, const std::string& key) {
  if (!params.count(key)) {
    throw std::runtime_error(std::format("Key \"{}\" not found in Params.", key));
  }
  return std::get<T>(params.at(key));
}

struct Color {
  float r;
  float g;
  float b;
  float w;
};

template <>
struct std::formatter<Color> : std::formatter<std::string> {
  auto format(const Color& c, std::format_context& ctx) const {
    return std::formatter<std::string>::format(
      std::format("{}, {}, {}, {}", c.r, c.g, c.b, c.w), ctx);
  }
};

class Shader {
  public:
    GLuint id;

    Shader(const char* vertex_src, const char* frag_src) {
      GLuint vs = glCreateShader(GL_VERTEX_SHADER);
      glShaderSource(vs, 1, &vertex_src, nullptr);
      glCompileShader(vs);

      GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
      glShaderSource(fs, 1, &frag_src, nullptr);
      glCompileShader(fs);

      GLint ok = 0;
      glGetShaderiv(vs, GL_COMPILE_STATUS, &ok);
      if (!ok) {
        GLint len = 0;
        glGetShaderiv(vs, GL_INFO_LOG_LENGTH, &len);
        std::vector<char> log(len);
        glGetShaderInfoLog(vs, len, nullptr, log.data());
        glDeleteShader(vs);
        throw std::runtime_error(std::format("Shader error: {}", log.data()));
      }

      ok = 0;
      glGetShaderiv(fs, GL_COMPILE_STATUS, &ok);
      if (!ok) {
        GLint len = 0;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &len);
        std::vector<char> log(len);
        glGetShaderInfoLog(fs, len, nullptr, log.data());
        glDeleteShader(fs);
        throw std::runtime_error(std::format("Shader error: {}", log.data()));
      }

      id = glCreateProgram();

      glAttachShader(id, vs);
      glAttachShader(id, fs);
      glBindAttribLocation(id, 0, "a_position");
      glBindAttribLocation(id, 1, "a_texcoord");
      glLinkProgram(id);

      glDeleteShader(vs);
      glDeleteShader(fs);

      ok = 0;
      glGetProgramiv(id, GL_LINK_STATUS, &ok);
      if (!ok) {
        GLint len = 0;
        glGetProgramiv(id, GL_INFO_LOG_LENGTH, &len);
        std::vector<char> log(len);
        glGetProgramInfoLog(id, len, nullptr, log.data());
        glDeleteProgram(id);
        throw std::runtime_error(std::format("Program link error: {}", log.data()));
      }
    }

    void use() const { 
      glUseProgram(id); 
    }

    void set_bool(const std::string &name, bool value) const {         
      glUniform1i(glGetUniformLocation(id, name.c_str()), (int)value); 
    }

    void set_int(const std::string &name, int value) const { 
      glUniform1i(glGetUniformLocation(id, name.c_str()), value); 
    }

    void set_float(const std::string &name, float value) const { 
      glUniform1f(glGetUniformLocation(id, name.c_str()), value); 
    }

    void set_float3(const std::string &name, float v1, float v2, float v3) const {
      glUniform3f(glGetUniformLocation(id, name.c_str()), v1, v2, v3); 
    }

    void set_float4(const std::string &name, float v1, float v2, float v3, float v4) const {
      glUniform4f(glGetUniformLocation(id, name.c_str()), v1, v2, v3, v4); 
    }
};

class Texture {
  public:
    static Shader shader;

    size_t n;
    size_t m;
    std::vector<float> texture;

    GLuint id;
    GLuint vbo;

    Texture(const std::vector<float>& data, size_t n, size_t m) : n(n), m(m), texture(data) {
      if (len() != data.size()) {
        throw std::runtime_error("Invalid texture dimensions passed to Texture.");
      }

      // Fullscreen quad (two triangles), with texture coords
      const GLfloat vertices[] = {
        // x,   y,   u,   v
        -1.f,-1.f, 0.f, 0.f,
        1.f,-1.f, 1.f, 0.f,
        -1.f, 1.f, 0.f, 1.f,
        1.f, 1.f, 1.f, 1.f
      };

      glGenBuffers(1, &vbo);
      glBindBuffer(GL_ARRAY_BUFFER, vbo);
      glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

      // Generate texture from float RGBA array
      glGenTextures(1, &id);
      glBindTexture(GL_TEXTURE_2D, id);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m, n, 0,
          GL_RGBA, GL_FLOAT, data.data());

      // Sampling settings
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    Texture(size_t n, size_t m) : Texture(std::vector<float>(4*n*m), n, m) {}

    Texture()=default;

    void set(size_t i, size_t j, Color color) {
      size_t idx = 4*(i + j*m);
      texture[idx]   = color.r;
      texture[idx+1] = color.g;
      texture[idx+2] = color.b;
      texture[idx+3] = color.w;
    }
    
    size_t len() const {
      return 4*n*m;
    }

    const float* data() const {
      return texture.data();
    }

    static Shader& get_shader() {
      static Shader* shader = nullptr;
      if (!shader) {

        const char* vertex_shader_src = R"(
        attribute vec2 a_position;
        attribute vec2 a_texcoord;
        varying vec2 v_texcoord;
        void main() {
            v_texcoord = a_texcoord;
            gl_Position = vec4(a_position, 0.0, 1.0);
        }
        )";

        const char* fragment_shader_src = R"(
        precision mediump float;
        varying vec2 v_texcoord;
        uniform sampler2D u_texture;
        void main() {
            gl_FragColor = texture2D(u_texture, v_texcoord);
        }
        )";

        shader = new Shader(vertex_shader_src, fragment_shader_src);
      }
      return *shader;
    }

    void draw() {
      Shader& shader = get_shader();
      shader.use();

      glBindBuffer(GL_ARRAY_BUFFER, vbo);

      glEnableVertexAttribArray(0); // position
      glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)0);

      glEnableVertexAttribArray(1); // texcoord
      glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));

      glBindTexture(GL_TEXTURE_2D, id);
      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m, n,
          GL_RGBA, GL_FLOAT, data());

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, id);

      GLint loc = glGetUniformLocation(shader.id, "u_texture");
      glUniform1i(loc, 0);

      // Draw 2 triangles as a strip
      glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

      glDisableVertexAttribArray(0);
      glDisableVertexAttribArray(1);
    }
};

class Simulator {
  public:
    Simulator()=default;

    virtual void timesteps(uint32_t num_steps)=0;

    virtual void key_callback(const std::string& key) {
      return;
    }

    virtual void draw() const {
      throw std::runtime_error("Called draw on a C++ simulator that does not implement it.");
    }
};
