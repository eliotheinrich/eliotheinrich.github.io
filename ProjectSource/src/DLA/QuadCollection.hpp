#pragma once

#include <vector>
#include <GLES3/gl3.h>
#include "Simulator.hpp" // for Color
const char* quad_vertex_src = R"(
attribute vec3 aPos;
void main() {
    gl_Position = vec4(aPos, 1.0);
}
)";

const char* quad_fragment_src = R"(
precision mediump float;

uniform vec4 color;

void main() {
    gl_FragColor = color;
}
)";


class QuadCollection {
  public:
    QuadCollection(const Shader* shader) : shader(shader), num_categories(0) {
      // Initialize buffers
      glGenVertexArrays(1, &VAO);
      glGenBuffers(1, &VBO);
      glGenBuffers(1, &EBO);

      // Configure vertex attributes
      glBindVertexArray(VAO);
      glBindBuffer(GL_ARRAY_BUFFER, VBO);

      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
      glEnableVertexAttribArray(0);

      // Unbind buffers
      glBindVertexArray(0);
      glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    ~QuadCollection() {
      glDeleteVertexArrays(1, &VAO);
      glDeleteBuffers(1, &VBO);
      glDeleteBuffers(1, &EBO);
    }

    void draw() {
      // Binding buffers
      glBindVertexArray(VAO);
      glBindBuffer(GL_ARRAY_BUFFER, VBO);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

      // Load vertex data into VBO/EBO
      glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(float), vertices.data(), GL_STREAM_DRAW);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(unsigned int), indices.data(), GL_STREAM_DRAW);
      
      shader->use();

      // Draw each category of vertex
      size_t n = 0;
      for (size_t i = 0; i < num_categories; i++) {
        shader->set_float4("color", colors[i].r, colors[i].g, colors[i].b, colors[i].w);

        glDrawElements(GL_TRIANGLES, index_counts[i], GL_UNSIGNED_INT, (void*)(n * sizeof(unsigned int)));
        n += index_counts[i];
      }

      // Unbind buffers
      glBindVertexArray(0);
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    void add_vertices(const std::vector<float>& new_vertices, const std::vector<unsigned int>& new_indices, Color color) {
      size_t num_old_vertices = vertices.size() / 3;
      // push vertices
      vertices.insert(vertices.end(), new_vertices.begin(), new_vertices.end());
      // push shifted indices
      for (size_t i = 0; i < new_indices.size(); i++) {
        indices.push_back(new_indices[i] + num_old_vertices);
      }

      index_counts.push_back(new_indices.size());
      colors.push_back(color);
      num_categories++;
    }

  private:
    const Shader* shader;

    unsigned int VAO;
    unsigned int VBO;
    unsigned int EBO;

    size_t num_categories;
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    std::vector<size_t> index_counts;
    std::vector<Color> colors;
};


inline std::pair<std::vector<float>, std::vector<unsigned int>> make_quad(
        float x, float y, float w, float h, unsigned int start_idx) {
    std::vector<float> vertices = {
        x,     y,     0.0f,
        x + w, y,     0.0f,
        x + w, y + h, 0.0f,
        x,     y + h, 0.0f
    };

    std::vector<unsigned int> indices = {
        start_idx, start_idx + 1, start_idx + 2,
        start_idx, start_idx + 2, start_idx + 3
    };

    return {vertices, indices};
}


