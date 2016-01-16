#pragma once

#include <vector>

#include "framebuffer.h"
#include "pipeline.h"

class Context {
 public:
  void setVertices(const std::vector<Vertex> &vert) { vertices_ = &vert; }
  void setFrameBuffer(FrameBuffer &fb) { fb_ = &fb; }
  void setVertexShader(VertexShader vs) { vs_ = vs; }
  void setFragmentShader(FragmentShader fs) { fs_ = fs; }
  void setWireframeMode(bool mode) { wireframe_ = mode; }
  void SetUniform(const Uniform &u) { uniform_ = &u; }
  void draw();

 protected:
  const std::vector<Vertex> *vertices_{nullptr};
  FrameBuffer *fb_{nullptr};
  VertexShader vs_{nullptr};
  FragmentShader fs_{nullptr};
  const Uniform *uniform_{nullptr};
  bool wireframe_{false};
};
