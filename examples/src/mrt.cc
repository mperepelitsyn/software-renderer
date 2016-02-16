#include <algorithm>

#include "app/app.h"
#include "app/obj_parser.h"
#include "app/tga_loader.h"
#include "renderer/texture.h"

using namespace renderer;
using namespace app;

constexpr auto width = 1200;
constexpr auto height = 900;

namespace {

struct DeferredStage1 : Program {
  struct Uniform {
    Mat4 mv;
    Mat4 mvp;
    Texture<UNorm> tex_diff;
    Texture<UNorm> *rt_color;
    Texture<Vec3> *rt_normal;
    Texture<Vec3> *rt_pos_v;
  };

  struct Attr {
    Vec3 normal;
    Vec3 pos_v;
    Vec2 tc;
  };

  static void vertexShader(const Vertex &in, const void *u, VertexH &out) {
    auto &vin = static_cast<const ObjVertex&>(in);
    auto &uin = *static_cast<const Uniform*>(u);
    auto &aout = *static_cast<Attr*>(out.attr);

    auto n = uin.mv * Vec4{vin.normal, 0.f};
    auto pv = uin.mv * Vec4{in.pos, 1.f};
    out.pos = uin.mvp * Vec4{in.pos, 1.f};
    aout.normal = {n.x, n.y, n.z};
    aout.pos_v = {pv.x, pv.y, pv.z};
    aout.tc = vin.tc;
  }

  static void fragmentShader(const Fragment &in, const void *u, Vec4 &) {
    auto &ain = *static_cast<const Attr*>(in.attr);
    auto &uin = *static_cast<const Uniform*>(u);
    auto x = in.coord.x;
    auto y = in.coord.y;

    uin.rt_color->setTexel(x, y, uin.tex_diff.sample(ain.tc.x, ain.tc.y));
    uin.rt_normal->setTexel(x, y, normalize(ain.normal));
    uin.rt_pos_v->setTexel(x, y, ain.pos_v);
  }

  DeferredStage1() : Program{vertexShader, fragmentShader, 8} {}
};

struct DeferredStage2 : Program {
  struct Uniform {
    const Texture<UNorm> *rt_color;
    const Texture<Vec3> *rt_normal;
    const Texture<Vec3> *rt_pos_v;
  };

  static void vertexShader(const Vertex &in, const void *, VertexH &out) {
    out = {{in.pos, 1.f}, {}};
  }

  static void fragmentShader(const Fragment &in, const void *u, Vec4 &out) {
    static const Vec3 to_light = normalize({0.5f, 1.f, 1.f});
    static const Vec4 ambient_albedo{.3f, .3f, .3f, 1.f};
    static const Vec4 diffuse_albedo{.7f, .7f, .7f, 1.f};
    static const Vec4 specular_albedo{.2f, .2f, .2f, 1.f};
    static constexpr auto spec_power = 64u;
    auto &uin = *static_cast<const Uniform*>(u);

    auto tex = uin.rt_color->fetchTexel(in.coord.x, in.coord.y);
    if (tex.a == 0.f) {
      out = tex;
      return;
    }
    auto n = uin.rt_normal->fetchTexel(in.coord.x, in.coord.y);
    auto pos_v = uin.rt_pos_v->fetchTexel(in.coord.x, in.coord.y);

    auto to_eye = normalize(-pos_v);
    auto ambient = ambient_albedo * tex;
    auto diffuse =  tex * std::max(dot(n, to_light), 0.f) * diffuse_albedo;
    auto specular = specular_albedo * std::pow(std::max(dot(
            reflect(-to_light, n), to_eye), 0.f), spec_power);

    out = ambient + diffuse + specular;
  }

  DeferredStage2() : Program{vertexShader, fragmentShader, 0} {}
};

} // namespace

class MRTApp : public App {
 public:
  MRTApp(unsigned w, unsigned h, const std::string &name)
    : App{w, h, name},
      model_{parseObj("../assets/stormtrooper.obj")},
      quad_{
        {{-1.f, -1.f, -1.f}}, {{1.f, -1.f, -1.f}}, {{-1.f, 1.f, -1.f}},
        {{-1.f, 1.f, -1.f}}, {{1.f, -1.f, -1.f}}, {{1.f, 1.f, -1.f}}
      },
      vb_model_{&model_[0], model_.size(), sizeof(model_[0])},
      vb_quad_{&quad_[0], quad_.size(), sizeof(quad_[0])},
      rt_color{w, h},
      rt_normal{w, h},
      rt_pos_v{w, h},
      uniform1_{{}, {},
        {1024, 1024, loadTGA("../assets/stormtrooper_d.tga")},
        &rt_color, &rt_normal, &rt_pos_v
      },
      uniform2_{&rt_color, &rt_normal, &rt_pos_v} {}

 private:
  void startup() override {
    ctx_.setCulling(Pipeline::BACK_FACING);

    rt_color.clear();
    rt_normal.clear();
    rt_pos_v.clear();

    proj_ = createPerspProjMatrix(70.0_deg,
        static_cast<float>(width_) / height_, 1.f, 100.f);
  }

  void renderLoop(double time, double) override {
    fb_.clear();
    rt_color.clear();
    rt_normal.clear();
    rt_pos_v.clear();

    float px = std::sin(time * 0.3f) * 3.5f;
    float py = std::cos(time * 0.3f) * 0.2f + 0.7f;
    float ty = std::sin(time * 0.3f) * 0.2f * -1.f - 2.1f;
    auto view = createViewMatrix({px, py, 8.f}, {px, ty, 0.f}, {0.f, 1.f, 0.f});

    fb_.setColorWrite(false);
    ctx_.setUniform(&uniform1_);
    ctx_.setVertexBuffer(&vb_model_);
    ctx_.setProgram(&prog1_);

    for (auto i = -5; i <= 5; i += 2) {
      for (auto j = -5; j <= 5; ++j) {
        auto model = translate(Vec3(i, 0.f, j));
        uniform1_.mv = view * model;
        uniform1_.mvp = proj_ * view * model;
        ctx_.draw();
      }
    }

    fb_.setColorWrite(true);
    ctx_.setVertexBuffer(&vb_quad_);
    ctx_.setUniform(&uniform2_);
    ctx_.setProgram(&prog2_);
    ctx_.draw();
  }

  std::vector<ObjVertex> model_;
  std::vector<Vertex> quad_;
  VertexBuffer vb_model_;
  VertexBuffer vb_quad_;
  Texture<UNorm> rt_color;
  Texture<Vec3> rt_normal;
  Texture<Vec3> rt_pos_v;
  DeferredStage1::Uniform uniform1_;
  DeferredStage2::Uniform uniform2_;
  DeferredStage1 prog1_;
  DeferredStage2 prog2_;
  Mat4 proj_;
};

DEFINE_AND_CALL_APP(MRTApp, width, height, Multiple Render Targets)


