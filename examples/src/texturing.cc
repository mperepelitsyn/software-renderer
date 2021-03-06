#include <algorithm>

#include "app/app.h"
#include "app/obj_parser.h"
#include "renderer/texture.h"

using namespace renderer;

namespace {

struct MyProgram : Program {
  struct Attr {
    Vec3 normal;
    Vec3 pos_v;
    Vec2 tc;
  };

  struct Uniform {
    Mat4 mv;
    Mat4 mvp;
    Texture<UNorm> tex;
  };

  static void vertexShader(const Vertex &in, const void *u, VertexH &out) {
    auto &vin = static_cast<const app::ObjVertex&>(in);
    auto &uin = *static_cast<const Uniform*>(u);
    auto &aout = *static_cast<Attr*>(out.attr);

    auto n = uin.mv * Vec4{vin.normal, 0.f};
    auto pv = uin.mv * Vec4{in.pos, 1.f};
    out.pos = uin.mvp * Vec4{in.pos, 1.f};
    aout.normal = {n.x, n.y, n.z};
    aout.pos_v = {pv.x, pv.y, pv.z};
    aout.tc = vin.tc;
  }

  static void fragmentShader(const Fragment &in, const void *u, Vec4 &out) {
    const static Vec3 to_light = normalize({0.5f, 1.f, 1.f});
    const static Vec4 ambient_albedo{.1f, .1f, .1f, 1.f};
    const static Vec4 diffuse_albedo{.7f, .7f, .7f, 1.f};
    const static Vec4 specular_albedo{.2f, .2f, .2f, 1.f};
    const static unsigned spec_power = 64;
    auto &fin = *static_cast<const Attr*>(in.attr);
    auto &uin = *static_cast<const Uniform*>(u);

    auto to_eye = normalize(-fin.pos_v);
    auto n = normalize(fin.normal);
    auto tex = uin.tex.sample(fin.tc.x, fin.tc.y);
    auto ambient = tex * ambient_albedo;
    auto diffuse =  tex * diffuse_albedo * std::max(dot(n, to_light), 0.f);
    auto specular = specular_albedo * std::pow(std::max(dot(
            reflect(-to_light, n), to_eye), 0.f), spec_power);

    out = ambient + diffuse + specular;
  }

  MyProgram() : Program{vertexShader, fragmentShader, 8} {}
};

auto genCheckerTexture(unsigned width, unsigned height, unsigned step) {
  std::vector<UNorm> out(width * height);
  bool white{true};

  for (auto r = 0u; r < height; ++r) {
    if (r % step == 0)
      white = !white;
    for (auto c = 0u; c < width; ++c) {
      if (c % step == 0)
        white = !white;
      out[r * width + c] = {
        static_cast<unsigned char>(white * 255),
        static_cast<unsigned char>(white * 255),
        static_cast<unsigned char>(white * 255),
        255
      };
    }
  }

  return out;
}

} // namespace

class TexturingApp : public app::App {
 public:
  TexturingApp(unsigned w, unsigned h, const std::string &name)
    : App{w, h, name},
      vertices_{app::parseObj("../assets/cube.obj")},
      vb_{&vertices_[0], vertices_.size(), sizeof(vertices_[0])},
      uniform_{{}, {}, {512, 512, genCheckerTexture(512, 512, 64)}} {}

 private:
  void startup() override {
    ctx_.setVertexBuffer(&vb_);
    ctx_.setProgram(&prog_);
    ctx_.setUniform(&uniform_);
    ctx_.setCulling(Pipeline::BACK_FACING);

    view_ = createViewMatrix({0.f, 0.f, 3.7f}, {0.f, 0.f, 0.f},
                             {0.f, 1.f, 0.f});
    auto proj = createPerspProjMatrix(70.0_deg,
        static_cast<float>(width_) / height_, 1.f, 100.f);
    proj_view_ = proj * view_;
  }

  void renderLoop(double time, double) override {
    fb_.clear();

    auto model = rotateY(time * .5f) * rotateZ(time * .2f);
    uniform_.mv = view_ * model;
    uniform_.mvp = proj_view_ * model;
    ctx_.draw();
  }

  std::vector<app::ObjVertex> vertices_;
  VertexBuffer vb_;
  MyProgram prog_;
  MyProgram::Uniform uniform_;
  Mat4 view_;
  Mat4 proj_view_;
};

DEFINE_AND_CALL_APP(TexturingApp, 1200, 900, Texturing)


