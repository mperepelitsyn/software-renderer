#include <algorithm>

#include "app/app.h"
#include "app/obj_parser.h"

using namespace renderer;

namespace {

struct MyProgram : Program {
  struct Attr {
    Vec3 normal;
    Vec3 pos_v;
  };

  struct Uniform {
    Mat4 mv;
    Mat4 mvp;
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
  }

  static void fragmentShader(const Fragment &in, const void *, Vec4 &out) {
    const static Vec3 to_light = normalize({0.5f, 1.f, 1.f});
    const static Vec3 ambient_albedo{.1f, .1f, .1f};
    const static Vec3 diffuse_albedo{.8f, .8f, .8f};
    const static Vec3 specular_albedo{.3f, .3f, .3f};
    const static unsigned spec_power = 64;
    auto &ain = *static_cast<const Attr*>(in.attr);

    auto to_eye = normalize(-ain.pos_v);
    auto n = normalize(ain.normal);
    auto diffuse = diffuse_albedo * std::max(dot(n, to_light), 0.f);
    auto specular = specular_albedo * std::pow(std::max(dot(
            reflect(-to_light, n), to_eye), 0.f), spec_power);

    out = {ambient_albedo + diffuse + specular, 1.f};
  }

  MyProgram() : Program{vertexShader, fragmentShader, 6} {}
};

} // namespace

class ZBufferApp : public app::App {
 public:
  using App::App;

 private:
  void startup() override {
    ctx_.setVertexBuffer(&vb_);
    ctx_.setProgram(&prog_);
    ctx_.setUniform(&uniform_);

    view_ = createViewMatrix({0.f, 1.1f, 3.7f}, {0.f, 1.f, 0.f},
                             {0.f, 1.f, 0.f});
    auto proj = createPerspProjMatrix(70.0_deg,
        static_cast<float>(width_) / height_, 1.f, 100.f);
    proj_view_ = proj * view_;
  }

  void renderLoop(double time, double) override {
    fb_.clear();

    auto model = rotateX(std::sin(time * 0.4f) * 0.15f + 0.2f) *
                 rotateY(time * .5f);
    uniform_.mv = view_ * model;
    uniform_.mvp = proj_view_ * model;
    ctx_.draw();
  }

  std::vector<app::ObjVertex> vertices_{app::parseObj("../assets/teapot.obj")};
  VertexBuffer vb_{&vertices_[0], static_cast<unsigned>(vertices_.size()),
                   sizeof(vertices_[0])};
  MyProgram prog_;
  MyProgram::Uniform uniform_;
  Mat4 view_;
  Mat4 proj_view_;
};

DEFINE_AND_CALL_APP(ZBufferApp, 1200, 900, ZBuffer)

