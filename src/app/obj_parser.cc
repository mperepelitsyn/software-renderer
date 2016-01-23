#include <fstream>
#include <sstream>
#include <tuple>

#include "app/obj_parser.h"

using namespace renderer;

namespace app {

namespace {

auto consumeFaceElement(std::istringstream &iss) {
  unsigned vertex{}, uv{}, normal{};
  iss >> vertex;
  if (iss.peek() == '/') {
    iss.get();
    if (iss.peek() != '/')
      iss >> uv;
  }
  if (iss.peek() == '/') {
    iss.get();
    iss >> normal;
  }
  return std::make_tuple(vertex, uv, normal);
}

} // namespace

std::vector<ObjVertex> parseObj(const std::string &path) {
  std::vector<ObjVertex> out;
  std::ifstream fs(path);
  if (!fs.good())
    return out;

  std::vector<Vec3> vertices;
  std::vector<Vec3> normals;
  std::vector<Vec2> uvs;

  std::string line, type;
  while (std::getline(fs, line)) {
    if (line.empty())
      continue;

    std::istringstream iss(line);
    iss >> type;

    if (type == "v") {
      float x, y, z;
      iss >> x >> y >> z;
      vertices.emplace_back(x, y, z);
    }
    else if (type == "vt") {
      float u, v;
      iss >> u >> v;
      uvs.emplace_back(u, v);
    }
    else if (type == "vn") {
      float x, y, z;
      iss >> x >> y >> z;
      normals.emplace_back(x, y, z);
    }
    else if (type == "f") {
      unsigned vertex_id, uv_id, normal_id;

      for (auto i = 0u; i < 3; ++i) {
        std::tie(vertex_id, uv_id, normal_id) = consumeFaceElement(iss);
        out.emplace_back(vertices[vertex_id - 1],
                         normal_id ? normals[normal_id - 1] : Vec3{},
                         uv_id ? uvs[uv_id - 1] : Vec2{});
      }
    }
  }

  return out;
}

} // namespace app
