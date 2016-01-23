#pragma once

#include <vector>

#include "renderer/framebuffer.h"
#include "renderer/pipeline.h"

namespace renderer {

class Context {
 public:
  void setVertexBuffer(const VertexBuffer *vb) { vb_ = vb; }
  void setFrameBuffer(FrameBuffer &fb) { fb_ = &fb; }
  void setWireframeMode(bool mode) { wireframe_ = mode; }
  void setUniform(const void *u) { uniform_ = u; }
  void setProgram(const Program *program) { program_ = program; }
  void draw();

 protected:
  const VertexBuffer *vb_{nullptr};
  FrameBuffer *fb_{nullptr};
  const Program *program_;
  const void *uniform_{nullptr};
  bool wireframe_{false};
};

} // namespace renderer
