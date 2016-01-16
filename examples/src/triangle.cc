#include "app/app.h"

using namespace renderer;

namespace {

void vertexShader(const Vertex &in, const Uniform &u, Vertex &out) {
  out.position = u.mvp * in.position;
  auto n = u.mvp * Vec4{in.normal, 0.f};
  out.normal = {n.x, n.y, n.z};
  out.color = in.color;
  out.tex_coord = in.tex_coord;
}

void fragmentShader(const Fragment &in, const Uniform &, Vec4 &out) {
  out = Vec4(in.color.r, in.color.g, in.color.b, 1.f);
}

} // namespace

class TriangleApp : public app::App {
 public:
  using App::App;

 private:
  void startup() override {
    std::vector<Vertex> vertices{
      {{-.5f, .5f, .0f, 1.f}, {1.f, .0f, .0f}},
      {{.0f, -.5f, .0f, 1.f}, {0.f, 0.f, 1.f}},
      {{.5f, .5f, .0f, 1.f}, {0.f, 1.f, 0.f}},
    };
    ctx_.setVertices(vertices);
    ctx_.setVertexShader(vertexShader);
    ctx_.setFragmentShader(fragmentShader);

    auto view = createViewMatrix({0.f, 0.f, 2.f}, {0.f, 0.f, 0.f},
                                 {0.f, 1.f, 0.f});
    auto proj = createPerspProjMatrix(70.0_deg,
        static_cast<float>(width_) / height_, 0.01f, 100.f);
    proj_view_ = proj * view;
  }

  void renderLoop(double time, double) override {
    fb_.clear();

    auto model = rotateZ(time) * translate({0.3f, 0.f, 0.f}) *
                 rotateZ(-time) * rotateY(time);
    ctx_.setUniform({proj_view_ * model});

    ctx_.draw();
  }

  Mat4 proj_view_;
};

DEFINE_AND_CALL_APP(TriangleApp, 640, 480, Triangle)
