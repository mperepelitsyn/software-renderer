#include "app/app.h"
#include "app/obj_parser.h"

using namespace renderer;

namespace {

void vertexShader(const Vertex &in, const Uniform &u, Vertex &out) {
  auto n = u.mv * Vec4{in.normal, 0.f};
  auto pv = u.mv * in.position;
  out.position = u.mvp * in.position;
  out.normal = {n.x, n.y, n.z};
  out.color = in.color;
  out.tex_coord = in.tex_coord;
  out.pos_view = {pv.x, pv.y, pv.z};
}

void fragmentShader(const Fragment &in, const Uniform &, Vec4 &out) {
  const static Vec3 to_light = normalize({0.5f, 1.f, 1.f});
  const static Vec3 ambient_albedo{.1f, .1f, .1f};
  const static Vec3 diffuse_albedo{.8f, .8f, .8f};
  const static Vec3 specular_albedo{1.f, 1.f, 1.f};
  const static float spec_power = 64;

  auto to_eye = normalize(in.pos_view * -1.f);
  auto n = normalize(in.normal);
  auto diffuse = diffuse_albedo * std::max(dot(n, to_light), 0.f);
  auto specular = specular_albedo * std::pow(std::max(dot(
          reflect(to_light * -1.f, n), to_eye), 0.f), spec_power);

  out = {ambient_albedo + diffuse + specular, 1.f};
}

} // namespace

class ZBufferApp : public app::App {
 public:
  using App::App;

 private:
  void startup() override {
    auto vertices = app::parseObj("../assets/teapot.obj");
    ctx_.setVertices(vertices);
    ctx_.setVertexShader(vertexShader);
    ctx_.setFragmentShader(fragmentShader);

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
    ctx_.setUniform({proj_view_ * model, view_ * model});
    ctx_.draw();
  }

  Mat4 view_;
  Mat4 proj_view_;
};

DEFINE_AND_CALL_APP(ZBufferApp, 1200, 900, ZBuffer)

