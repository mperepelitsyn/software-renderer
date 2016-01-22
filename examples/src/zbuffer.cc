#include "app/app.h"
#include "app/obj_parser.h"

using namespace renderer;

namespace {

struct Uniform {
  Mat4 mv;
  Mat4 mvp;
};

struct MyAttrs : public Attrs {
  Vec3 normal;
  Vec3 pos_v;
};

void vertexShader(const Vertex &in, const void *u, VertexH &out) {
  auto ain = static_cast<const app::ObjAttrs*>(in.attrs.get());
  auto uin = static_cast<const Uniform*>(u);
  auto aout = std::make_unique<MyAttrs>();

  auto n = uin->mv * Vec4{ain->normal, 0.f};
  auto pv = uin->mv * Vec4{in.pos, 1.f};
  out.pos = uin->mvp * Vec4{in.pos, 1.f};
  aout->normal = {n.x, n.y, n.z};
  aout->pos_v = {pv.x, pv.y, pv.z};
  out.attrs.reset(aout.release());
}

void fragmentShader(const Fragment &in, const void *, Vec4 &out) {
  const static Vec3 to_light = normalize({0.5f, 1.f, 1.f});
  const static Vec3 ambient_albedo{.1f, .1f, .1f};
  const static Vec3 diffuse_albedo{.8f, .8f, .8f};
  const static Vec3 specular_albedo{.3f, .3f, .3f};
  const static unsigned spec_power = 64;
  auto ain = static_cast<const MyAttrs*>(in.attrs.get());

  auto to_eye = normalize(ain->pos_v * -1.f);
  auto n = normalize(ain->normal);
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
    vertices_ = app::parseObj("../assets/teapot.obj");
    ctx_.setVertices(&vertices_);
    ctx_.setVertexShader(vertexShader);
    ctx_.setFragmentShader(fragmentShader);
    ctx_.setUniform(&uniform_);
    ctx_.setAttrSize(sizeof(MyAttrs));

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

  Mat4 view_;
  Mat4 proj_view_;
  std::vector<Vertex> vertices_;
  Uniform uniform_;
};

DEFINE_AND_CALL_APP(ZBufferApp, 1200, 900, ZBuffer)

