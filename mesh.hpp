#include <QVector3D>
#include <array>
#include <istream>
#include <vector>

using point_id = int;
using halfedge_id = int;
using edge_id = int;
using triangle_id = int;

struct halfedge {
    halfedge(point_id start, point_id end)
        : start{start}, end{end}, opposite{-1}, edge{-1}
    {}

    point_id start;
    point_id end;
    halfedge_id opposite;
    edge_id edge;
};

struct mesh {
    std::vector<QVector3D> points;
    std::vector<std::array<point_id, 3>> triangles;
};

mesh parse_obj(std::istream &stream);

struct navigatable_mesh : mesh {
    navigatable_mesh(mesh m);
    std::vector<halfedge> halfedges;
    std::vector<QVector3D> point_normals;
    std::vector<halfedge_id> point_to_halfedge;
    std::vector<halfedge_id> edge_to_halfedge;

    triangle_id halfedge_to_triangle(halfedge_id e);
    halfedge_id triangle_to_halfedge(halfedge_id e);
    halfedge_id next_around_triangle(halfedge_id e);
    halfedge_id next_around_point(halfedge_id e);
    QVector3D halfedge_vector(halfedge_id e);
    QVector3D triangle_normal(triangle_id t);
    float triangle_area(triangle_id t);
};