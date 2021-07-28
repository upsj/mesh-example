#include <QVector3D>
#include <istream>
#include <vector>

struct edge {
  edge(int a, int b)
      : points{std::min(a, b), std::max(a, b)}, triangles{-1, -1},
        triangle_starts{-1, -1} {}
  std::array<int, 2> points;
  std::array<int, 2> triangles;
  std::array<int, 2> triangle_starts;
};

struct triangle {
  triangle(std::array<int, 3> points) : points{points}, edges{-1, -1, -1} {}
  std::array<int, 3> points;
  std::array<int, 3> edges;
};

struct mesh {
  std::vector<QVector3D> points;
  std::vector<std::array<int, 3>> triangles;
};

struct navigatable_mesh {
  navigatable_mesh(mesh m);
  std::vector<QVector3D> points;
  std::vector<triangle> triangles;
  std::vector<edge> edges;
  std::vector<int> point_neighborhood_offsets;
  std::vector<int> point_neighbors;
  std::vector<int> point_triangles;
};

mesh parse_obj(std::istream &stream);