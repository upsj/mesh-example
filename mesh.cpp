#include "mesh.hpp"

#include <limits>
#include <map>
#include <sstream>
#include <string>

mesh parse_obj(std::istream &stream) {
  mesh result;
  std::string line;
  while (std::getline(stream, line)) {
    if (!line.empty()) {
      std::stringstream ss{line};
      std::string mode;
      ss >> mode;
      if (mode == "v") {
        double x{};
        double y{};
        double z{};
        ss >> x >> y >> z;
        result.points.emplace_back(std::array<double, 3>{x, y, z});
      } else if (mode == "f") {
        int a{};
        int b{};
        int c{};
        ss >> a >> b >> c;
        result.triangles.emplace_back(std::array<int, 3>{a - 1, b - 1, c - 1});
      }
    }
  }
  return result;
}

vtkNew<vtkPolyData> mesh::to_vtk() const {
  vtkNew<vtkPoints> out_points;
  out_points->SetNumberOfPoints(points.size());
  for (vtkIdType i = 0; i < points.size(); i++) {
    auto point = points[i];
    out_points->SetPoint(i, point[0], point[1], point[2]);
  }
  vtkNew<vtkCellArray> out_triangles;
  for (vtkIdType i = 0; i < triangles.size(); i++) {
    auto triangle = triangles[i];
    out_triangles->InsertNextCell(3);
    out_triangles->InsertCellPoint(triangle[0]);
    out_triangles->InsertCellPoint(triangle[1]);
    out_triangles->InsertCellPoint(triangle[2]);
  }
  vtkNew<vtkPolyData> out;
  out->SetPoints(out_points);
  out->SetPolys(out_triangles);
  return out;
}

navigatable_mesh::navigatable_mesh(mesh m) : mesh{std::move(m)} {
  halfedges.reserve(triangles.size() * 3);
  edge_to_halfedge.reserve(triangles.size() * 3 / 2);
  point_to_halfedge.assign(points.size(),
                           std::numeric_limits<halfedge_id>::max());
  std::map<std::pair<point_id, point_id>, edge_id> edge_map;
  for (auto tri : triangles) {
    auto insert_halfedge = [&](point_id u, point_id v) {
      auto e_pair = std::make_pair(std::min(u, v), std::max(u, v));
      auto it = edge_map.find(e_pair);
      edge_id e_idx{-1};
      halfedge_id he_idx = halfedges.size();
      halfedges.emplace_back(u, v);
      auto &he = halfedges.back();
      if (it == edge_map.end()) {
        // add a new edge, link edge -> halfedge
        e_idx = edge_to_halfedge.size();
        it = edge_map.emplace_hint(it, e_pair, e_idx);
        edge_to_halfedge.emplace_back(he_idx);
      } else {
        // use existing edge, link halfedge <-> halfedge
        e_idx = it->second;
        auto other_he_idx = edge_to_halfedge[e_idx];
        auto &other_he = halfedges[other_he_idx];
        he.opposite = other_he_idx;
        other_he.opposite = he_idx;
      }
      // link halfedge -> edge
      halfedges.back().edge = e_idx;
      // link points -> halfedge
      point_to_halfedge[u] = std::min(point_to_halfedge[u], he_idx);
      point_to_halfedge[v] = std::min(point_to_halfedge[v], he_idx);
    };
    insert_halfedge(tri[0], tri[1]);
    insert_halfedge(tri[1], tri[2]);
    insert_halfedge(tri[2], tri[0]);
  }
}

triangle_id navigatable_mesh::halfedge_to_triangle(halfedge_id e) const {
  assert(e >= 0 && e < halfedges.size());
  return e / 3;
}

halfedge_id navigatable_mesh::triangle_to_halfedge(triangle_id t) const {
  assert(t >= 0 && t < triangles.size());
  return t * 3;
}

halfedge_id navigatable_mesh::next_around_triangle(halfedge_id e) const {
  assert(e >= 0 && e < halfedges.size());
  return (e / 3 * 3) + (e % 3 + 1) % 3;
}

halfedge_id navigatable_mesh::next_around_point(halfedge_id e) const {
  assert(e >= 0 && e < halfedges.size());
  return halfedges[(e / 3 * 3) + (e % 3 + 2) % 3].opposite;
}