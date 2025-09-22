#pragma once

#include "DiffusionPhysicsModel.hpp"
#include "QuadCollection.hpp"

#define CLUSTER_POINT -1

struct DLALightningModelConfig {
  size_t n;
  size_t m;

  bool insert_left;
  bool insert_right;
  bool insert_top;
  bool insert_bottom;

  bool can_insert;

  double initial_density;

  size_t bbox_left;
  size_t bbox_right;
  size_t bbox_bottom;
  size_t bbox_top;
  int bbox_margin;

  std::string initial_state;

  int seed;

  DLALightningModelConfig()=default;

  static DLALightningModelConfig from_json(nlohmann::json& json) {
    DLALightningModelConfig config;

    config.n = static_cast<size_t>(json.value("n", 100));
    config.m = static_cast<size_t>(json.value("m", config.n));

    config.initial_density = static_cast<double>(json.value("initial_density", 0.0));

    config.insert_left = static_cast<bool>(json.value("insert_left", true));
    config.insert_right = static_cast<bool>(json.value("insert_right", true));
    config.insert_top = static_cast<bool>(json.value("insert_top", true));
    config.insert_bottom = static_cast<bool>(json.value("insert_bottom", true));

    config.bbox_margin = static_cast<size_t>(json.value("bbox_margin", 10));

    config.initial_state = static_cast<std::string>(json.value("initial_state", "central_cluster"));

    int seed = static_cast<int>(json.value("seed", -1));
    if (seed == -1) {
      thread_local std::random_device rd;
      config.seed = rd();
    } else {
      config.seed = seed;
    }

    return config;
  }
};

class DLALightningModel : public DiffusionPhysicsModel {
  public:
    std::vector<int> field;
    std::vector<Point> insertions;
    size_t frame;
    std::vector<Texture> lightning_textures;

    DLALightningModel(const DLALightningModelConfig& config) : DiffusionPhysicsModel(config.seed) {
      n = config.n;
      m = config.m;

      insert_left   = config.insert_left;
      insert_right  = config.insert_right;
      insert_top    = config.insert_top;
      insert_bottom = config.insert_bottom;

      bbox_left   = config.bbox_left;
      bbox_right  = config.bbox_right;
      bbox_top    = config.bbox_top;
      bbox_bottom = config.bbox_bottom;
      bbox_margin = config.bbox_margin;

      target_particle_density = config.initial_density;

      field = std::vector<int>(n*m);

      std::string initial_state = config.initial_state;
      if (initial_state == "central_cluster") {
        initialize_central_cluster();
      } else if (initial_state == "boundary_cluster") {
        initialize_boundary_cluster();
      }

      shader = Shader("vertex_shader.vs", "fragment_shader.fs");

      can_insert = true;
      fix_bbox();
      init_keymap();

      frame = 0;

      init_textures();
    }

    virtual void set_cluster_color(Color color) override { 
      cluster_color = color;
    }

    virtual void set_particle_color(Color color) override { }

    virtual Texture get_cluster_texture() const override {
      return lightning_textures[frame];
    }

    virtual Texture get_particle_texture() const override {
      return Texture(n, m);
    }

    virtual void diffusion_step() override {
      frame = (frame + 1) % lightning_textures.size();
    }

    bool contains_boundary(const std::vector<Point>& points) {
      for (const auto &p : points) {
        if (p.x == 0 || p.x == n -1 || p.y == 0 || p.y == m - 1) {
          return true;
        }
      }

      return false;
    }

    std::vector<Point> get_insertion_order() {
      bool searching = true;
      size_t iter = 0;

      std::vector<Point> insertions;

      while (searching) {
        std::vector<Point> new_points = _diffusion_step();
        insertions.insert(insertions.end(), new_points.begin(), new_points.end());

        if (contains_boundary(new_points)) {
          searching = false;
        }

        iter++;
        if (iter > 1000) {
          searching = false;
        }
      }

      return insertions;
    }

    void init_textures() {
      std::vector<Point> insertions = get_insertion_order();
      std::reverse(insertions.begin(), insertions.end());

      Texture last(n, m);
      for (auto const& p : insertions) {
        Texture texture(last);
        Color c = {1.0, 1.0, 1.0, 1.0};
        texture.set(p.x, p.y, c);
        lightning_textures.push_back(texture);

        last = texture;
      }
    }

    std::vector<Point> _diffusion_step() {
      double particle_density = 0.0;
      double cluster_size;

      std::vector<Point> new_points;

      for (size_t i = 0; i < n; i++) {
        for (size_t j = 0; j < m; j++) {
          Point site(i, j);
          if (get(site) == CLUSTER_POINT) {
            cluster_size += 1.0;
          } else {
            int num_particles = get(site);
            particle_density += num_particles;
            for (size_t k = 0; k < num_particles; k++) {
              std::vector<Point> targets = neighbors(site, true);
              size_t target_idx = rng() % targets.size();
              Point target = targets[target_idx];

              // Remove particle from site and either freeze it into the cluster or move it to a neighboring site
              get(site)--;
              bool freeze_target = has_cluster_neighbor(target);
              if (freeze_target) {
                insert_cluster(target);
                new_points.push_back(target);
              } else {
                get(target)++;
              }
            }
          }
        }
      }

      size_t bbox_width = bbox_right - bbox_left;
      size_t bbox_height = bbox_top - bbox_bottom;
      double available_volume = bbox_width*bbox_height - cluster_size;
      particle_density = particle_density / available_volume;

      int num_new_particles = (target_particle_density - particle_density)*available_volume;
      for (int i = 0; i < num_new_particles; i++) {
        insert_particle();
      }

      return new_points;
    }

    void insert_particle() {
      if (!can_insert) {
        return;
      }

      // Need to add new particle
      std::vector<size_t> directions;
      if (insert_left) {
        directions.push_back(0);
      } if (insert_right) {
        directions.push_back(1);
      } if (insert_top) {
        directions.push_back(2);
      } if (insert_bottom) {
        directions.push_back(3);
      }

      // If insertions are not allowed at the boundary, randomly choose a point
      Point site;
      size_t bbox_width = bbox_right - bbox_left;
      size_t bbox_height = bbox_top - bbox_bottom;
      if (directions.size() == 0) {
        site = Point(rng() % bbox_width + bbox_left, rng() % bbox_height + bbox_bottom);

        size_t niter = 100;
        size_t k = 0;
        while (has_cluster_neighbor(site) && k < niter) {
          site = Point(rng() % bbox_width + bbox_left, rng() % bbox_height + bbox_bottom);
          k++;
        } 

        if (k == niter) {
          can_insert = false;
          return;
        }
        
      } else {
        size_t direction = directions[rng() % directions.size()];
        if (direction == 0) { // left
          site = Point(bbox_left, rng() % bbox_height + bbox_bottom);
        } else if (direction == 1) { // right
          site = Point(bbox_right, rng() % bbox_height + bbox_bottom);
        } else if (direction == 2) { // bottom
          site = Point(rng() % bbox_width + bbox_left, bbox_top);
        } else if (direction == 3) { // top
          site = Point(rng() % bbox_width + bbox_left, bbox_bottom);
        }
      } 

      insert_particle(site);
    }

    virtual void insert_particle(const Point& p) override {
      if (get(p) == CLUSTER_POINT) {
        return;
      }

      get(p)++;
    }

    virtual void key_callback(GLFWwindow *window, int key, int action) override {
      if (keymap.contains(key)) {
        double time = glfwGetTime();
        if (update_key_time(key, time) > INPUT_DELAY) {
          auto func = keymap[key];
          func(*this, window);
        }
      }
    }

    void init_keymap() {
    }

  private:
    KeyMap<DLALightningModel> keymap;

    size_t n;
    size_t m;

    double target_particle_density;

    bool insert_left;
    bool insert_right;
    bool insert_top;
    bool insert_bottom;
    bool can_insert;

    size_t bbox_left;
    size_t bbox_right;
    size_t bbox_bottom;
    size_t bbox_top;
    size_t bbox_margin;

    Shader shader;
    Color cluster_color;

    void fix_bbox() {
      bbox_left = n-1;
      bbox_right = 0;
      bbox_top = 0;
      bbox_bottom = m-1;

      bool found_cluster = false;
      for (size_t x = 0; x < n; x++) {
        for (size_t y = 0; y < m; y++) {
          Point p(x, y);
          if (get(p) == CLUSTER_POINT) {
            update_bbox(p);
            found_cluster = true;
          }
        }
      }

      if (!found_cluster) {
        int center = n/2;
        bbox_left = (n - bbox_margin)/2;
        bbox_right = (n + bbox_margin)/2;
        bbox_bottom = (m - bbox_margin)/2;
        bbox_top = (m + bbox_margin)/2;
      }
    }

    void update_bbox(const Point& p) {
      int x_left =   std::max(int(p.x) - int(bbox_margin), int(0));
      int x_right =  std::min(int(p.x) + int(bbox_margin), int(n-1));
      int y_bottom = std::max(int(p.y) - int(bbox_margin), int(0));
      int y_top =    std::min(int(p.y) + int(bbox_margin), int(m-1));

      if (x_right > bbox_right) {
        bbox_right = x_right;
      }

      if (x_left < bbox_left) {
        bbox_left = x_left;
      }

      if (y_top > bbox_top) {
        bbox_top = y_top;
      }

      if (y_bottom < bbox_bottom) {
        bbox_bottom = y_bottom;
      }
    }

    void initialize_central_cluster() {
      Point center(n/2, m/2);
      insert_cluster(center);
    }

    virtual void insert_cluster(const Point& p) override {
      if (get(p) == CLUSTER_POINT) {
        return;
      }

      get(p) = CLUSTER_POINT;
      update_bbox(p);
    }

    void initialize_boundary_cluster() {
      for (size_t i = 0; i < n; i++) {
        Point p(i, 0);
        insert_cluster(p);
      }

      insert_bottom = false;
      insert_left = false;
      insert_right = false;
      insert_top = true;
    }

    int& operator[](const Point& p) {
      return field[p.to_index(n, m)];
    }

    int operator[](const Point& p) const {
      return field[p.to_index(n, m)];
    }

    int& get(const Point& p) {
      return operator[](p);
    }

    int get(const Point& p) const {
      return operator[](p);
    }

    bool within_bbox(const Point& p) const {
      return p.x >= bbox_left && p.x <= bbox_right & p.y >= bbox_bottom & p.y <= bbox_top;
    }

    std::vector<Point> neighbors(const Point& p, bool bounded=false) const {
      size_t left = bounded ? bbox_left : 0;
      size_t right = bounded ? bbox_right : n-1;
      size_t bottom = bounded ? bbox_bottom : 0;
      size_t top = bounded ? bbox_top : m-1;

      std::vector<Point> points;
      if (p.x != left) {
        points.push_back(Point(p.x-1, p.y));
      }

      if (p.x != right) {
        points.push_back(Point(p.x+1, p.y));
      }

      if (p.y != bottom) {
        points.push_back(Point(p.x, p.y-1));
      }

      if (p.y != top) {
        points.push_back(Point(p.x, p.y+1));
      }

      return points;
    }

    bool has_cluster_neighbor(const Point &p) {
      if (get(p) == CLUSTER_POINT) {
        return true;
      }

      std::vector<Point> target_neighbors = neighbors(p);
      for (auto const &t : target_neighbors) {
        if (get(t) == CLUSTER_POINT) {
          return true;
        }
      }

      return false;
    }

    std::pair<std::vector<float>, std::vector<unsigned int>> get_bbox_vertices(bool active) const {
      std::vector<float> vertices;
      std::vector<unsigned int> indices;

      std::vector<float> new_vertices;
      std::vector<unsigned int> new_indices;

      float thickness = 0.01;

      float left = ((float) bbox_left/n - 0.5)*2.0;
      float right = ((float) bbox_right/n - 0.5)*2.0;
      float top = ((float) bbox_top/m - 0.5)*2.0;
      float bottom = ((float) bbox_bottom/m - 0.5)*2.0;

      float width = right - left;
      float height = top - bottom;

      // Bottom
      if (insert_bottom == active) {
        std::tie(new_vertices, new_indices) = make_quad(left, bottom, width, thickness, vertices.size() / 3);
        vertices.insert(vertices.end(), new_vertices.begin(), new_vertices.end());
        indices.insert(indices.end(), new_indices.begin(), new_indices.end());
      }

      // Left
      if (insert_left == active) {
        std::tie(new_vertices, new_indices) = make_quad(left, bottom, thickness, height, vertices.size() / 3);
        vertices.insert(vertices.end(), new_vertices.begin(), new_vertices.end());
        indices.insert(indices.end(), new_indices.begin(), new_indices.end());
      }

      // Top
      if (insert_top == active) {
        std::tie(new_vertices, new_indices) = make_quad(left, top-thickness, width, thickness, vertices.size() / 3);
        vertices.insert(vertices.end(), new_vertices.begin(), new_vertices.end());
        indices.insert(indices.end(), new_indices.begin(), new_indices.end());
      }

      // Right
      if (insert_right == active) {
        std::tie(new_vertices, new_indices) = make_quad(right-thickness, bottom, thickness, height, vertices.size() / 3);
        vertices.insert(vertices.end(), new_vertices.begin(), new_vertices.end());
        indices.insert(indices.end(), new_indices.begin(), new_indices.end());
      }

      return {vertices, indices};
    }
};
