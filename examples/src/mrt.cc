#include <map>

#include "app/app.h"
#include "app/obj_parser.h"
#include "app/tga_loader.h"
#include "renderer/texture.h"

using namespace renderer;
using namespace app;

constexpr auto width = 1200;
constexpr auto height = 900;

namespace {

struct Uniform {
  Mat4 mv;
  Mat4 mvp;
  Texture tex_diff;
  Texture rt_color;
  Texture rt_normal;
  Texture rt_pos_v;
};

struct DeferredStage1 : Program {
  struct MyVertexH : VertexH {
    Vec3 normal;
    Vec3 pos_v;
    Vec2 tc;
  };

  struct MyFragment : Fragment {
    Vec3 normal;
    Vec3 pos_v;
    Vec2 tc;
  };

  static void vertexShader(const Vertex &in, const void *u, VertexH &out) {
    auto &vin = static_cast<const ObjVertex&>(in);
    auto uin = static_cast<const Uniform*>(u);
    auto &vout = static_cast<MyVertexH&>(out);

    auto n = uin->mv * Vec4{vin.normal, 0.f};
    auto pv = uin->mv * Vec4{in.pos, 1.f};
    vout.pos = uin->mvp * Vec4{in.pos, 1.f};
    vout.normal = {n.x, n.y, n.z};
    vout.pos_v = {pv.x, pv.y, pv.z};
    vout.tc = vin.tc;
  }

  static void fragmentShader(const Fragment &in, const void *u, Vec3 *out) {
    auto &fin = static_cast<const MyFragment&>(in);
    auto uin = static_cast<const Uniform*>(u);

    out[0] = uin->tex_diff.sample(fin.tc.x, fin.tc.y);
    out[1] = normalize(fin.normal);
    out[2] = fin.pos_v;
  }

  DeferredStage1() : Program{vertexShader, fragmentShader, 8, 3} {}
};

struct DeferredStage2 : Program {
  static void vertexShader(const Vertex &in, const void *, VertexH &out) {
    out = {{in.pos, 1.f}};
  }

  static void fragmentShader(const Fragment &in, const void *u, Vec3 *out) {
    const static Vec3 to_light = normalize({0.5f, 1.f, 1.f});
    const static Vec3 specular_albedo{.2f, .2f, .2f};
    const static unsigned spec_power = 64;
    auto uin = static_cast<const Uniform*>(u);

    auto tex = uin->rt_color.fetchTexel(in.coord.x, in.coord.y);
    auto n = uin->rt_normal.fetchTexel(in.coord.x, in.coord.y);
    auto pos_v = uin->rt_pos_v.fetchTexel(in.coord.x, in.coord.y);

    auto to_eye = normalize(pos_v * -1.f);
    auto diffuse =  tex * std::max(dot(n, to_light), 0.f);
    auto specular = specular_albedo * std::pow(std::max(dot(
            reflect(to_light * -1.f, n), to_eye), 0.f), spec_power);

    *out = {diffuse + specular};
  }

  DeferredStage2() : Program{vertexShader, fragmentShader, 0, 1} {}
};

} // namespace

class MRTApp : public App {
 public:
  using App::App;

 private:
  void startup() override {
    ctx_.setUniform(&uniform_);
    ctx_.setCulling(Pipeline::BACK_FACING);

    gbuffer_.attachColor(0, &uniform_.rt_color);
    gbuffer_.attachColor(1, &uniform_.rt_normal);
    gbuffer_.attachColor(2, &uniform_.rt_pos_v);

    proj_ = createPerspProjMatrix(70.0_deg,
        static_cast<float>(width_) / height_, 1.f, 100.f);
  }

  void renderLoop(double time, double) override {
    fb_.clear();
    gbuffer_.clear();

    float px = std::sin(time * 0.3f) * 3.5f;
    float py = std::cos(time * 0.3f) * 0.2f + 0.7f;
    float ty = std::sin(time * 0.3f) * 0.2f * -1.f - 2.1f;
    auto view = createViewMatrix({px, py, 8.f}, {px, ty, 0.f}, {0.f, 1.f, 0.f});

    ctx_.setFrameBuffer(&gbuffer_);
    ctx_.setVertexBuffer(&vb_model_);
    ctx_.setProgram(&stage1_);

    for (auto i = -5; i <= 5; i += 2) {
      for (auto j = -5; j <= 5; ++j) {
        auto model = translate(Vec3(i, 0.f, j));
        uniform_.mv = view * model;
        uniform_.mvp = proj_ * view * model;
        ctx_.draw();
      }
    }

    ctx_.setFrameBuffer(backbuffer_);
    ctx_.setVertexBuffer(&vb_quad_);
    ctx_.setProgram(&stage2_);
    ctx_.draw();
  }

  std::vector<ObjVertex> model_{parseObj("../assets/stormtrooper.obj")};
  std::vector<Vertex> quad_{
    {{-1.f, -1.f, 0.f}}, {{1.f, -1.f, 0.5f}}, {{-1.f, 1.f, 0.5f}},
    {{-1.f, 1.f, 0.5f}}, {{1.f, -1.f, 0.5f}}, {{1.f, 1.f, 0.5f}}
  };
  VertexBuffer vb_model_{&model_[0], static_cast<unsigned>(model_.size()),
                         sizeof(model_[0])};
  VertexBuffer vb_quad_{&quad_[0], static_cast<unsigned>(quad_.size()),
                        sizeof(quad_[0])};
  DeferredStage1 stage1_;
  DeferredStage2 stage2_;
  Uniform uniform_{{}, {},
    {1024, 1024, loadTGA("../assets/stormtrooper_d.tga")},
    {width, height}, {width, height}, {width, height}};
  FrameBuffer gbuffer_{width, height, 3};
  FrameBuffer *backbuffer_{ctx_.getFrameBuffer()};
  Mat4 proj_;
};

DEFINE_AND_CALL_APP(MRTApp, width, height, Multiple Render Targets)


