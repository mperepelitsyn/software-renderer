#include "app/app.h"

using namespace renderer;

namespace {

struct MyVertex : Vertex {
  MyVertex(const Vec3 &pos, const Vec3 &color) : Vertex{pos}, color{color} {}
  Vec3 color;
};

struct MyProgram : Program {
  struct MyVertexH : VertexH {
    Vec3 color;
  };

  struct MyFragment : Fragment {
    Vec3 color;
  };

  struct Uniform {
    Mat4 mvp;
  };

  static void vertexShader(const Vertex &in, const void *u, VertexH &out) {
    auto &vin = static_cast<const MyVertex&>(in);
    auto uin = static_cast<const Uniform*>(u);
    auto &vout = static_cast<MyVertexH&>(out);

    vout.pos = uin->mvp * Vec4{in.pos, 1.f};
    vout.color = vin.color;
  }

  static void fragmentShader(const Fragment &in, const void *, Vec4 &out) {
    auto &fin = static_cast<const MyFragment&>(in);
    out = Vec4(fin.color, 1.f);
  }

  MyProgram() : Program{vertexShader, fragmentShader, 3} {}
};


} // namespace

class TriangleApp : public app::App {
 public:
  using App::App;

 private:
  void startup() override {
    ctx_.setVertexBuffer(&vb_);
    ctx_.setProgram(&prog_);
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

  std::vector<MyVertex> vertices_{
        {{-.5f, .5f, .0f}, {1.f, .0f, .0f}},
        {{.0f, -.5f, .0f}, {0.f, 1.f, .0f}},
        {{.5f, .5f, .0f}, {0.f, .0f, 1.f}},
      };
  VertexBuffer vb_{&vertices_[0],
    static_cast<unsigned>(vertices_.size()), sizeof(MyVertex)};
  Mat4 proj_view_;
  MyProgram::Uniform uniform_;
  MyProgram prog_;
};

DEFINE_AND_CALL_APP(TriangleApp, 640, 480, Triangle)
