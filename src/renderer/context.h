#pragma once

#include <vector>

#include "renderer/framebuffer.h"
#include "renderer/pipeline.h"

namespace renderer {

class Context {
 public:
  void setVertices(const std::vector<Vertex> *vert) { vertices_ = vert; }
  void setFrameBuffer(FrameBuffer &fb) { fb_ = &fb; }
  void setVertexShader(VertexShader vs) { vs_ = vs; }
  void setFragmentShader(FragmentShader fs) { fs_ = fs; }
  void setWireframeMode(bool mode) { wireframe_ = mode; }
  void setUniform(const void *u) { uniform_ = u; }
  void setAttrSize(unsigned sz) { attr_size_ = sz; }
  void draw();

 protected:
  const std::vector<Vertex> *vertices_{nullptr};
  FrameBuffer *fb_{nullptr};
  VertexShader vs_{nullptr};
  FragmentShader fs_{nullptr};
  const void *uniform_{nullptr};
  unsigned attr_size_{};
  bool wireframe_{false};
};

} // namespace renderer
