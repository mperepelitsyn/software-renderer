#include "app/app.h"
#include "app/obj_parser.h"

using namespace renderer;

namespace {

struct MyProgram : Program {
  struct Uniform {
    Mat4 mvp;
  };

  static void vertexShader(const Vertex &in, const void *u, VertexH &out) {
    auto uin = static_cast<const Uniform*>(u);
    out.pos = uin->mvp * Vec4{in.pos, 1.f};
  }

  static void fragmentShader(const Fragment &, const void *, Vec3 *out) {
    *out = {1.f, 1.f, 1.f};
  }

  MyProgram() : Program{vertexShader, fragmentShader, 0, 1} {}
};


} // namespace

class CullingApp : public app::App {
 public:
  using App::App;

 private:
  void startup() override {
    ctx_.setVertexBuffer(&vb_);
    ctx_.setProgram(&prog_);
    ctx_.setUniform(&uniform_);
    ctx_.setWireframeMode(true);

    auto view = createViewMatrix({0.f, 0.f, 5.5f}, {0.f, 0.f, 0.f},
                                 {0.f, 1.f, 0.f});
    auto proj = createPerspProjMatrix(70.0_deg,
        static_cast<float>(width_) / height_, 0.01f, 100.f);
    proj_view_ = proj * view;
  }

  void renderLoop(double time, double) override {
    fb_.clear();

    uniform_.mvp = proj_view_ * model_[0] * rotateY(time * 0.3f);
    ctx_.setCulling(Pipeline::BACK_FACING);
    ctx_.draw();

    uniform_.mvp = proj_view_ * rotateY(time * 0.3f);
    ctx_.setCulling(Pipeline::NONE);
    ctx_.draw();

    uniform_.mvp = proj_view_ * model_[1] * rotateY(time * 0.3f);
    ctx_.setCulling(Pipeline::FRONT_FACING);
    ctx_.draw();
  }

  std::vector<app::ObjVertex> vertices_{app::parseObj("../assets/monkey.obj")};
  VertexBuffer vb_{&vertices_[0], static_cast<unsigned>(vertices_.size()),
                   sizeof(vertices_[0])};
  Mat4 model_[2]{translate({-3.f, 0.f, 0.f}), translate({3.f, 0.f, 0.f})};
  Mat4 proj_view_;
  MyProgram::Uniform uniform_;
  MyProgram prog_;
};

DEFINE_AND_CALL_APP(CullingApp, 1200, 900, Culling)
