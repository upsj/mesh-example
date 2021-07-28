#include "mesh.hpp"

#include <sstream>
#include <string>
#include <map>
#include <limits>

mesh parse_obj(std::istream &stream)
{
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

navigatable_mesh::navigatable_mesh(mesh m) : mesh{std::move(m)}
{
    halfedges.reserve(triangles.size() * 3);
    edge_to_halfedge.reserve(triangles.size() * 3 / 2);
    point_to_halfedge.assign(points.size(),
                             std::numeric_limits<halfedge_id>::max());
    normals.resize(points.size());
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
    // compute normals
    for (point_id i = 0; i < points.size(); i++) {
        QVector3D area_weighted_normal_sum{};
        auto first_e = point_to_halfedge[i];
        auto e = first_e;
        while (next_around_point(e) != first_e) {
            auto t = halfedge_to_triangle(e);
            auto area = triangle_area(t);
            auto normal = triangle_normal(t);
            area_weighted_normal_sum += area * normal;
        }
        normals[i] = area_weighted_normal_sum.normalized();
    }
}

triangle_id navigatable_mesh::halfedge_to_triangle(halfedge_id e)
{
    assert(e >= 0 && e < halfedges.size());
    return e / 3;
}

halfedge_id navigatable_mesh::triangle_to_halfedge(triangle_id t)
{
    assert(t >= 0 && t < triangles.size());
    return t * 3;
}

halfedge_id navigatable_mesh::next_around_triangle(halfedge_id e)
{
    assert(e >= 0 && e < halfedges.size());
    return (e / 3 * 3) + (e % 3 + 1) % 3;
}

halfedge_id navigatable_mesh::next_around_point(halfedge_id e)
{
    assert(e >= 0 && e < halfedges.size());
    return halfedges[(e / 3 * 3) + (e % 3 + 2) % 3].opposite;
}

QVector3D navigatable_mesh::halfedge_vector(halfedge_id e)
{
    return points[halfedges[e].end] - points[halfedges[e].start];
}

QVector3D navigatable_mesh::triangle_normal(triangle_id t)
{
    auto uv = halfedge_vector(halfedge_to_triangle(t));
    auto vw = halfedge_vector(halfedge_to_triangle(t) + 1);
    return QVector3D::normal(uv, vw);
}

float navigatable_mesh::triangle_area(triangle_id t)
{
    auto uv = halfedge_vector(halfedge_to_triangle(t));
    auto vw = halfedge_vector(halfedge_to_triangle(t) + 1);
    return QVector3D::crossProduct(uv, vw).length() / 2;
}