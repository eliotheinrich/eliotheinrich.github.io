#pragma once

#include "Simulator.hpp"

#include <stdexcept>
#include <unistd.h>
#include <future>
#include <vector>
#include <map>
#include <fstream>
#include <algorithm>
#include <cctype>


#include "Random.hpp"
#include "QuadCollection.hpp"

#define CLUSTER_POINT -1

Color DEFAULT_PARTICLE_COLOR = {1.0, 1.0, 1.0, 0.5};

Color BBOX_COLOR_ACTIVE = {1.0, 0.5, 0.0, 1.0};
Color BBOX_COLOR_INACTIVE = {0.5, 0.5, 0.0, 1.0};

std::vector<Color> DEFAULT_CLUSTER_COLORS = {
  {1.0, 0.0, 0.0, 1.0},
  {0.0, 1.0, 0.0, 1.0},
  {0.0, 0.0, 1.0, 1.0}
};

struct Point {
  int x;
  int y;

  Point()=default;
  Point(int x, int y) : x(x), y(y) {}

  inline size_t to_index(size_t n, size_t m) const {
    return x + n*y;
  }

  inline static size_t to_index(size_t x, size_t y, size_t n, size_t m) {
    return x + n*y;
  }

  inline static size_t mod(int i, int n) {
    return (i % n + n) % n;
  }

  inline static Point get_point_from_pos(double x, double y, size_t n, size_t m) {
    x = (x + 1.0)/2.0;
    y = (y + 1.0)/2.0;

    return Point(int(x*n), int(y*m));
  }
};

class DiffusionController : public Simulator {
  public:
    size_t n;
    size_t m;

    std::vector<int> field;
    Texture cluster_texture;
    Params initial_params;

    DiffusionController(Params& params) : initial_params(params), shader(quad_vertex_src, quad_fragment_src) {
      n = ::get<int>(params, "n");
      m = ::get<int>(params, "m");

      show_particles = ::get<int>(params, "show_particles", 0);

      cluster_colors = DEFAULT_CLUSTER_COLORS;
      particle_color = DEFAULT_PARTICLE_COLOR;

      color_idx = 0;

      paused = false;

      set_cluster_color(cluster_colors[color_idx]);
      set_particle_color(particle_color);

      insert_left   = ::get<int>(params, "insert_left", 1);
      insert_right  = ::get<int>(params, "insert_right", 1);
      insert_top    = ::get<int>(params, "insert_top", 1);
      insert_bottom = ::get<int>(params, "insert_bottom", 1);

      mobile_left  = ::get<int>(params, "mobile_left", 1);
      mobile_right = ::get<int>(params, "mobile_right", 1);
      mobile_up    = ::get<int>(params, "mobile_up", 1);
      mobile_down  = ::get<int>(params, "mobile_down", 1);

      bbox_left   = ::get<int>(params, "bbox_left", 1);
      bbox_right  = ::get<int>(params, "bbox_right", 1);
      bbox_top    = ::get<int>(params, "bbox_top", 1);
      bbox_bottom = ::get<int>(params, "bbox_bottom", 1);
      bbox_margin = ::get<int>(params, "bbox_margin", 10);

      show_bbox = ::get<int>(params, "show_bbox", 0);

      target_particle_density = ::get<double>(params, "target_particle_density", 0.0);

      pbc = ::get<int>(params, "pbc", 0);

      field = std::vector<int>(n*m);
      cluster_texture = Texture(n, m);

      std::string initial_state = ::get<std::string>(params, "initial_state", "central_cluster");
      if (initial_state == "central_cluster") {
        initialize_central_cluster();
      } else if (initial_state == "boundary_cluster") {
        initialize_boundary_cluster();
      }

      can_insert = true;
      fix_bbox();

      add_mouse_callbacks();
    }

    enum IntersectionDirection { Right, Left, Top, Bottom };
    IntersectionDirection invert(IntersectionDirection dir) const {
      if (dir == IntersectionDirection::Right) {
        return IntersectionDirection::Left;
      } else if (dir == IntersectionDirection::Left) {
        return IntersectionDirection::Right;
      } else if (dir == IntersectionDirection::Bottom) {
        return IntersectionDirection::Top;
      } else {
        return IntersectionDirection::Bottom;
      }
    }

    struct Intersection {
      IntersectionDirection direction;
      float x;
      float y;
    };

    // Given three collinear points, the function checks if point q lies on line segment 'pr'
    bool on_segment(float p_x, float p_y, float q_x, float q_y, float r_x, float r_y) {
      if (q_x <= std::max(p_x, r_x) && q_x >= std::min(p_x, r_x) &&
          q_y <= std::max(p_y, r_y) && q_y >= std::min(p_y, r_y)) {
        return true;
      }
      return false;
    }

    // 0 if p, q, and r are collinear
    // 1 if they are clockwise
    // 2 if they are counterclockwise
    int orientation(float p_x, float p_y, float q_x, float q_y, float r_x, float r_y) const {
      float val = (q_y - p_y) * (r_x - q_x) - (q_x - p_x) * (r_y - q_y);
      if (val == 0)  {
        return 0;  // Collinear
      } else {
        return (val > 0) ? 1 : 2; // Clockwise or counterclockwise
      }
    }

    // Function to check if segments 'p1q1' and 'p2q2' intersect
    std::optional<std::pair<float, float>> segment_intersection(float p1_x, float p1_y, float q1_x, float q1_y, float p2_x, float p2_y, float q2_x, float q2_y) const {
      int o1 = orientation(p1_x, p1_y, q1_x, q1_y, p2_x, p2_y);
      int o2 = orientation(p1_x, p1_y, q1_x, q1_y, q2_x, q2_y);
      int o3 = orientation(p2_x, p2_y, q2_x, q2_y, p1_x, p1_y);
      int o4 = orientation(p2_x, p2_y, q2_x, q2_y, q1_x, q1_y);

      // General case
      if (o1 != o2 && o3 != o4) {
        float x_intersect = ((p1_x * q1_y - p1_y * q1_x) * (p2_x - q2_x) - (p1_x - q1_x) * (p2_x * q2_y - p2_y * q2_x)) /
          ((p1_x - q1_x) * (p2_y - q2_y) - (p1_y - q1_y) * (p2_x - q2_x));
        float y_intersect = ((p1_x * q1_y - p1_y * q1_x) * (p2_y - q2_y) - (p1_y - q1_y) * (p2_x * q2_y - p2_y * q2_x)) /
          ((p1_x - q1_x) * (p2_y - q2_y) - (p1_y - q1_y) * (p2_x - q2_x));
        return std::make_pair(x_intersect, y_intersect);
      }

      // If none of the cases
      return std::nullopt;
    }



    std::vector<Intersection> grid_intersection(const Point& p, float x1, float y1, float x2, float y2) const {
      float tile_width = 2.0/n;
      float tile_height = 2.0/m;

      float left = ((float) p.x / n - 0.5)*2.0;
      float bottom = ((float) p.y / m - 0.5)*2.0;
      float right = left + tile_width;
      float top = bottom + tile_height;

      std::vector<Intersection> intersections;

      // Bottom
      auto bottom_intersection = segment_intersection(left, bottom, right, bottom, x1, y1, x2, y2);
      if (bottom_intersection.has_value()) {
        auto [x, y] = bottom_intersection.value();
        intersections.push_back({IntersectionDirection::Bottom, x, y});
      }

      // Top
      auto top_intersection = segment_intersection(left, top, right, top, x1, y1, x2, y2);
      if (top_intersection.has_value()) {
        auto [x, y] = top_intersection.value();
        intersections.push_back({IntersectionDirection::Top, x, y});
      }

      // Left
      auto left_intersection = segment_intersection(left, bottom, left, top, x1, y1, x2, y2);
      if (left_intersection.has_value()) {
        auto [x, y] = left_intersection.value();
        intersections.push_back({IntersectionDirection::Left, x, y});
      }

      // Right
      auto right_intersection = segment_intersection(right, bottom, right, top, x1, y1, x2, y2);
      if (right_intersection.has_value()) {
        auto [x, y] = right_intersection.value();
        intersections.push_back({IntersectionDirection::Right, x, y});
      }

      return intersections;
    }

    std::vector<Point> get_points_between(float x1, float y1, float x2, float y2) const {
      Point p1 = Point::get_point_from_pos(x1, y1, n, m);
      Point p2 = Point::get_point_from_pos(x2, y2, n, m);

      if (p1.x == p2.x && p1.y == p2.y) {
        return std::vector<Point>{p1};
      }

      std::vector<Point> points{p1};
      Point p = p1;

      float endpoint_x = x1;
      float endpoint_y = y1;
      std::vector<Intersection> intersections = grid_intersection(p, endpoint_x, endpoint_y, x2, y2);
      if (intersections.size() != 1) {
        throw std::invalid_argument("gridpoint p1 has multiple intersections\n");
      }

      auto [direction, x, y] = intersections[0];

      bool keep_going = true;
      while (keep_going) {
        if (direction == IntersectionDirection::Bottom) {
          p = Point(p.x, p.y-1);
        } else if (direction == IntersectionDirection::Top) {
          p = Point(p.x, p.y+1);
        } else if (direction == IntersectionDirection::Left) {
          p = Point(p.x-1, p.y);
        } else if (direction == IntersectionDirection::Right) {
          p = Point(p.x+1, p.y);
        }

        points.push_back(p);
        intersections = grid_intersection(p, endpoint_x, endpoint_y, x2, y2);

        if (intersections.size() == 2) {
          endpoint_x = (intersections[0].x + intersections[1].x)/2.0;
          endpoint_y = (intersections[0].y + intersections[1].y)/2.0;

          if (intersections[0].direction != invert(direction)) {
            direction = intersections[0].direction;
          } else {
            direction = intersections[1].direction;
          }
        } else if (intersections.size() == 1) {
          keep_going = false;
        } else {
          std::string error_message = "Error with grid tracing; found " + std::to_string(intersections.size()) + " intersections.";
          throw std::runtime_error(error_message);
        }
      }

      return points;
    }
    
    //void add_mouse_button_callback(GLFWwindow* window) {
    //  glfwSetWindowUserPointer(window, static_cast<void*>(this));

    //  auto mouse_callback = [](GLFWwindow* window, int button, int action, int mods) {
    //    auto self = static_cast<DiffusionController*>(glfwGetWindowUserPointer(window));
    //    glfwSetInputMode(window, GLFW_STICKY_MOUSE_BUTTONS, GLFW_TRUE);

    //    auto mouse_hover_event = [window, self]() {
    //      bool last_pos_exists = false;
    //      float xl, yl;
    //      while (self->lbutton_down) {
    //        int width, height;
    //        glfwGetWindowSize(window, &width, &height);

    //        double xpos, ypos;
    //        glfwGetCursorPos(window, &xpos, &ypos);
    //        xpos = (xpos/width - 0.5)*2.0;
    //        ypos = (-ypos/height + 0.5)*2.0;


    //        std::vector<Point> points;
    //        if (last_pos_exists) {
    //          points = self->get_points_between(xpos, ypos, xl, yl);
    //        } else {
    //          points = {Point::get_point_from_pos(xpos, ypos, self->n, self->m)};
    //        }

    //        for (auto const& p : points) {
    //          self->physics_model->insert_cluster(p);
    //        }

    //        xl = xpos;
    //        yl = ypos;
    //        last_pos_exists = true;
    //      }
    //    };


    //    if (button == GLFW_MOUSE_BUTTON_LEFT) {
    //      if (action == GLFW_PRESS) {
    //        self->lbutton_down = true;
    //        hover_future = std::async(std::launch::async, mouse_hover_event); 
    //      } else if (action == GLFW_RELEASE) {
    //        self->lbutton_down = false;
    //      }
    //    }

    //    return;
    //  };

    //  glfwSetMouseButtonCallback(window, mouse_callback);
    //}

    void advance_color() {
      color_idx = (color_idx + 1) % cluster_colors.size();
      set_cluster_color(cluster_colors[color_idx]);
    }

    virtual void timesteps(uint32_t num_steps) override {
      if (paused) {
        return;
      }

      for (size_t i = 0; i < num_steps; i++) {
        diffusion_step();
      }
    }

    Texture get_texture() const {
      return get_cluster_texture();
    }

    Texture get_cluster_texture() const {
      return cluster_texture;
    }

    Texture get_particle_texture() const {
      Texture particle_texture(n, m);
      for (size_t x = 0; x < n; x++) {
        for (size_t y = 0; y < m; y++) {
          Point p(x, y);
          if (get(p) > 0) {
            particle_texture.set(x, y, particle_color);
          }
        }
      }

      return particle_texture;
    }

    void diffusion_step() {
      double particle_density = 0.0;
      double cluster_size = 0.0;
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
              size_t target_idx = randi(0, targets.size());
              Point target = targets[target_idx];

              // Remove particle from site and either freeze it into the cluster or move it to a neighboring site
              get(site)--;
              bool freeze_target = has_cluster_neighbor(target);
              if (freeze_target) {
                insert_cluster(target);
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
    }

    void insert_cluster(const Point& p) {
      if (get(p) == CLUSTER_POINT) {
        return;
      }

      get(p) = CLUSTER_POINT;
      update_bbox(p);

      cluster_texture.set(p.x, p.y, cluster_color);
    }

    void insert_particle() {
      if (!can_insert) {
        return;
      }

      // Need to add new particle
      std::vector<size_t> directions;
      if (insert_left) {
        directions.push_back(0);
      } 
      if (insert_right) {
        directions.push_back(1);
      } 
      if (insert_top) {
        directions.push_back(2);
      } 
      if (insert_bottom) {
        directions.push_back(3);
      }

      // If insertions are not allowed at the boundary, randomly choose a point
      Point site;
      if (directions.size() == 0) {
        site = Point(randi(bbox_left, bbox_right), randi(bbox_bottom, bbox_top));

        size_t niter = 100;
        size_t k = 0;
        while (has_cluster_neighbor(site) && k < niter) {
          site = Point(randi(bbox_left, bbox_right), randi(bbox_bottom, bbox_top));
          k++;
        } 

        if (k == niter) {
          can_insert = false;
          return;
        }
        
      } else {
        size_t direction = directions[randi(0, directions.size())];
        if (direction == 0) { // left
          site = Point(bbox_left, randi(bbox_bottom, bbox_top));
        } else if (direction == 1) { // right
          site = Point(bbox_right, randi(bbox_bottom, bbox_top));
        } else if (direction == 2) { // bottom
          site = Point(randi(bbox_left, bbox_right), bbox_bottom);
        } else if (direction == 3) { // top
          site = Point(randi(bbox_left, bbox_right), bbox_top);
        }
      } 

      insert_particle(site);
    }

    void insert_particle(const Point& p) {
      if (get(p) == CLUSTER_POINT) {
        return;
      }

      get(p)++;
    }

    void set_cluster_color(Color color) {
      cluster_color = color;
    }

    void set_particle_color(Color color) { 
      particle_color = color;
    }

    static std::string to_upper(const std::string& str) {
      std::string s = str;
      std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return std::toupper(c); });
      return s;
    }

    virtual void key_callback(const std::string& key) override {
      std::string upper_key = to_upper(key);
      if (upper_key == "ARROWLEFT") {
        insert_left = !insert_left;
      } else if (upper_key == "ARROWRIGHT") {
        insert_right = !insert_right;
      } else if (upper_key == "ARROWUP") {
        insert_top = !insert_top;
      } else if (upper_key == "ARROWDOWN") {
        insert_bottom = !insert_bottom;
      } else if (upper_key == "B") {
        show_bbox = !show_bbox;
      } else if (upper_key == "E") {
        target_particle_density = std::min(1.0, target_particle_density + 0.01); 
      } else if (upper_key == "D") {
        target_particle_density = std::max(0.0, target_particle_density - 0.01);
      } else if (upper_key == "P") {
        show_particles = !show_particles;
      } else if (upper_key == "C") {
        advance_color();
      } else if (upper_key == "R") { // reset
        DiffusionController new_model(initial_params);
        new_model.show_particles = show_particles;
        new_model.show_bbox = show_bbox;
        *this = new_model;
      } else if (upper_key == " ") {
        paused = !paused;
      }
    }

    virtual void draw() const override {
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

      Texture cluster_texture = get_cluster_texture();
      cluster_texture.draw();

      if (show_particles) {
        Texture particle_texture = get_particle_texture();
        particle_texture.draw();
      }


      if (show_bbox) {
        QuadCollection quads(&shader);

        auto [bbox_vertices_inactive, bbox_indices_inactive] = get_bbox_vertices(false);
        quads.add_vertices(bbox_vertices_inactive, bbox_indices_inactive, BBOX_COLOR_INACTIVE);

        auto [bbox_vertices_active, bbox_indices_active] = get_bbox_vertices(true);
        quads.add_vertices(bbox_vertices_active, bbox_indices_active, BBOX_COLOR_ACTIVE);

        quads.draw();
      }

      glDisable(GL_BLEND);
    }

    // Mouse state tracking
    bool lbutton_down = false;

  private:
    Shader shader;
    double target_particle_density;

    bool insert_left;
    bool insert_right;
    bool insert_top;
    bool insert_bottom;
    bool can_insert;

    bool mobile_left;
    bool mobile_right;
    bool mobile_down;
    bool mobile_up;

    size_t bbox_left;
    size_t bbox_right;
    size_t bbox_bottom;
    size_t bbox_top;
    size_t bbox_margin;

    bool show_bbox;

    bool pbc;

    std::vector<Color> cluster_colors;
    size_t color_idx;
    Color cluster_color;
    Color particle_color;
    bool show_particles;

    std::vector<size_t> colors;

    bool paused;


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
      return p.x >= bbox_left && p.x <= bbox_right && p.y >= bbox_bottom && p.y <= bbox_top;
    }

    std::vector<Point> neighbors(const Point& p, bool bounded=false) const {
      size_t left = bounded ? bbox_left : 0;
      size_t right = bounded ? bbox_right : n-1;
      size_t bottom = bounded ? bbox_bottom : 0;
      size_t top = bounded ? bbox_top : m-1;

      std::vector<Point> points;
      if (pbc) {
        if (mobile_left) {
          points.push_back(Point((p.x == left) ? right-1 : p.x-1, p.y));
        }

        if (mobile_right) {
          points.push_back(Point((p.x == right) ? left+1 : p.x+1, p.y));
        }

        if (mobile_down) {
          points.push_back(Point(p.x, (p.y == bottom) ? top-1 : p.y-1));
        }

        if (mobile_up) {
          points.push_back(Point(p.x, (p.y == top) ? bottom+1 : p.y+1));
        }
      } else {
        if (p.x != left && mobile_left) {
          points.push_back(Point(p.x-1, p.y));
        }

        if (p.x != right && mobile_right) {
          points.push_back(Point(p.x+1, p.y));
        }

        if (p.y != bottom && mobile_down) {
          points.push_back(Point(p.x, p.y-1));
        }

        if (p.y != top && mobile_up) {
          points.push_back(Point(p.x, p.y+1));
        }
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
      float right = ((float) (bbox_right + 1)/n - 0.5)*2.0;
      float bottom = ((float) bbox_bottom/m - 0.5)*2.0;
      float top = ((float) (bbox_top + 1)/m - 0.5)*2.0;

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

    void add_mouse_callbacks();
};

float last_mouse_x = 0.0f;
float last_mouse_y = 0.0f;
bool last_pos_exists = false;


EM_BOOL mouse_down_callback(int eventType, const EmscriptenMouseEvent *e, void *userData) {
  std::cout << "Called mouse down callback\n";
  DiffusionController* self = static_cast<DiffusionController*>(userData);

  if (eventType == EMSCRIPTEN_EVENT_MOUSEDOWN && e->button == 0) { // Left button
    self->lbutton_down = true;
    last_pos_exists = false;
  } else if (eventType == EMSCRIPTEN_EVENT_MOUSEUP && e->button == 0) {
    self->lbutton_down = false;
    last_pos_exists = false;
  }
  return EM_TRUE;
}

EM_JS(void, get_element_position, (const char* selector, double* x, double* y), {
  var element = document.querySelector(UTF8ToString(selector));
  if (element) {
    var rect = element.getBoundingClientRect();
    HEAPF64[(x)>>3] = rect.left;
    HEAPF64[(y)>>3] = rect.top;
  } else {
    HEAPF64[(x)>>3] = -1;
    HEAPF64[(y)>>3] = -1;
  }
});

EM_BOOL mouse_move_callback(int eventType, const EmscriptenMouseEvent *e, void *userData) {
  std::cout << "Called mouse move callback\n";
  DiffusionController* self = static_cast<DiffusionController*>(userData);

  if (!self->lbutton_down) {
    last_pos_exists = false;
    return EM_TRUE;
  }

  double canvas_left = 0.0, canvas_top = 0.0;
  get_element_position("#canvas", &canvas_left, &canvas_top);

  double canvas_width, canvas_height;
  emscripten_get_element_css_size("#canvas", &canvas_width, &canvas_height);

  float ypos = -((e->clientY - canvas_top) / canvas_height - 0.5f) * 2.0f;
  float xpos = ((e->clientX - canvas_left) / canvas_width - 0.5f) * 2.0f;

  std::vector<Point> points;
  if (last_pos_exists) {
    points = self->get_points_between(xpos, ypos, last_mouse_x, last_mouse_y);
  } else {
    points = {Point::get_point_from_pos(xpos, ypos, self->n, self->m)};
  }

  for (auto const& p : points) {
    self->insert_cluster(p);
  }

  last_mouse_x = xpos;
  last_mouse_y = ypos;
  last_pos_exists = true;

  return EM_TRUE;
}

void DiffusionController::add_mouse_callbacks() {
  std::cout << "Added mouse callback\n";
  emscripten_set_mousedown_callback("#canvas", this, EM_TRUE, mouse_down_callback);
  emscripten_set_mouseup_callback("#canvas", this, EM_TRUE, mouse_down_callback);
  emscripten_set_mousemove_callback("#canvas", this, EM_TRUE, mouse_move_callback);
}
