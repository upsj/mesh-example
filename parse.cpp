#include "mesh.hpp"

#include <sstream>
#include <string>
#include <unordered_map>

struct plain_edge {
  plain_edge(int a, int b) : min(std::min(a, b)), max(std::max(a, b)) {}
  int min;
  int max;
  friend bool operator==(plain_edge a, plain_edge b) {
    return std::tie(a.min, a.max) == std::tie(b.min, b.max);
  }
  friend bool operator!=(plain_edge a, plain_edge b) {
    return std::tie(a.min, a.max) != std::tie(b.min, b.max);
  }
};

namespace std {
template <> struct hash<plain_edge> {
  std::size_t operator()(plain_edge const &e) const noexcept {
    return e.min + (std::size_t(e.max) << 32);
  }
};
} // namespace std

mesh parse_obj(std::istream &stream) {
  mesh result;
  std::string line;
  while (std::getline(stream, line)) {
    if (!line.empty()) {
      std::stringstream ss{line};
      std::string mode;
      ss >> mode;
      if (mode == "v") {
        float x{};
        float y{};
        float z{};
        ss >> x >> y >> z;
        result.points.emplace_back(x, y, z);
      } else if (mode == "f") {
        int a{};
        int b{};
        int c{};
        ss >> a >> b >> c;
        result.triangles.emplace_back(std::array<int, 3>{a, b, c});
      }
    }
  }
  return result;
}

navigatable_mesh::navigatable_mesh(mesh m) : points{std::move(m.points)} {
  // build references triangle <-> edge
  triangles.reserve(m.triangles.size());
  std::vector<int> first_edge_for_point(points.size(),
                                        std::numeric_limits<int>::max());
  std::unordered_map<plain_edge, int> edge_map;
  for (int i = 0; i < m.triangles.size(); i++) {
    triangles.emplace_back(m.triangles[i]);
    auto &triangle = triangles[i];
    // inserts an edge from the current triangle
    auto insert_edge = [&](int a, int b, int pos) {
      plain_edge check{a, b};
      auto it = edge_map.find(check);
      int result_edge{-1};
      if (it == edge_map.end()) {
        // half-edge not yet present: insert a new one
        result_edge = edges.size();
        edge_map[check] = result_edge;
        edges.emplace_back(a, b);
        edges.back().triangles[0] = i;
        edges.back().triangle_starts[0] = pos;
      } else {
        // else: extend half-edge by other half, check consistency
        result_edge = it->second;
        auto &cur = edges[it->second];
        auto old_tri = cur.triangles[0];
        assert(cur.triangles[0] != -1 && "Missing triangle reference");
        assert(cur.triangles[1] == -1 && "Non-manifold edge");
        assert(cur.triangle_starts[0] != -1 && "Broken triangle reference");
        assert(cur.triangle_starts[1] == -1 && "Broken triangle reference");
        cur.triangles[1] = i;
        cur.triangle_starts[1] = pos;
        auto old_start = cur.triangle_starts[0];
        auto old_a = triangles[old_tri].points[old_start];
        auto old_b = triangles[old_tri].points[(old_start + 1) % 3];
        assert((old_a == b && old_b == a) ||
               (old_a == a && old_b == b) &&
                   "Incorrect triangle start or reference");
        assert(old_a == b && old_b == a && "Winding order wrong");
      }
      // link point -> edge.
      first_edge_for_point[a] = std::min(first_edge_for_point[a], result_edge);
      first_edge_for_point[b] = std::min(first_edge_for_point[b], result_edge);
      return result_edge;
    };
    triangle.edges[0] = insert_edge(triangle.points[0], triangle.points[1], 0);
    triangle.edges[1] = insert_edge(triangle.points[1], triangle.points[2], 1);
    triangle.edges[2] = insert_edge(triangle.points[2], triangle.points[0], 2);
  }
  for (int i = 0; i < points.size(); i++) {
    // TODO: walk around the point using the triangles
  }
}