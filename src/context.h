#pragma once

#include <vector>

#include "framebuffer.h"
#include "pipeline.h"

class Context {
 public:
  Context() : vertices_{nullptr} {}
  void setVertices(const std::vector<Vertex> &vert) { vertices_ = &vert; }
  void setFrameBuffer(FrameBuffer &fb) { fb_ = &fb; }
  void setVertexShader(VertexShader vs) { vs_ = vs; }
  void setFragmentShader(FragmentShader fs) { fs_ = fs; }
  void setWireframeMode(bool mode) { wireframe_ = mode; }
  void draw();

 protected:
  const std::vector<Vertex> *vertices_;
  FrameBuffer *fb_;
  VertexShader vs_;
  FragmentShader fs_;
  bool wireframe_{false};
};
