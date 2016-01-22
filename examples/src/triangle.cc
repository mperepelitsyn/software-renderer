#include "app/app.h"

using namespace renderer;

namespace {

struct MyAttrs : public Attrs {
  MyAttrs(const Vec3 &color) : color{color} {}
  Vec3 color;
};

struct Uniform {
  Mat4 mvp;
};

void vertexShader(const Vertex &in, const void *u, VertexH &out) {
  auto ain = static_cast<const MyAttrs*>(in.attrs.get());
  auto uin = static_cast<const Uniform*>(u);

  out.attrs = std::make_unique<MyAttrs>(*ain);
  out.pos = uin->mvp * Vec4{in.pos, 1.f};
}

void fragmentShader(const Fragment &in, const void *, Vec4 &out) {
  auto ain = static_cast<const MyAttrs*>(in.attrs.get());
  out = Vec4(ain->color, 1.f);
}

} // namespace

class TriangleApp : public app::App {
 public:
  using App::App;

 private:
  void startup() override {
    Vertex init[] = {
      {{-.5f, .5f, .0f}, std::make_unique<MyAttrs>(Vec3{1.f, .0f, .0f})},
      {{.0f, -.5f, .0f}, std::make_unique<MyAttrs>(Vec3{0.f, 1.f, .0f})},
      {{.5f, .5f, .0f}, std::make_unique<MyAttrs>(Vec3{0.f, .0f, 1.f})},
    };

    vertices_.insert(vertices_.begin(),
                     std::make_move_iterator(std::begin(init)),
                     std::make_move_iterator(std::end(init)));
    ctx_.setVertices(&vertices_);
    ctx_.setAttrSize(3);
    ctx_.setVertexShader(vertexShader);
    ctx_.setFragmentShader(fragmentShader);
    ctx_.setUniform(&uniform_);

    auto view = createViewMatrix({0.f, 0.f, 2.f}, {0.f, 0.f, 0.f},
                                 {0.f, 1.f, 0.f});
    auto proj = createPerspProjMatrix(70.0_deg,
        static_cast<float>(width_) / height_, 0.01f, 100.f);
    proj_view_ = proj * view;
  }

  void renderLoop(double time, double) override {
    fb_.clear();

    auto model = rotateZ(time) * translate({0.3f, 0.f, 0.f}) *
                 rotateZ(-time * 1.2f);
    uniform_.mvp = proj_view_ * model;

    ctx_.draw();
  }

  Mat4 proj_view_;
  Uniform uniform_;
  std::vector<Vertex> vertices_;
};

DEFINE_AND_CALL_APP(TriangleApp, 640, 480, Triangle)
