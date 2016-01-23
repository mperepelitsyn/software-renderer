#include <cassert>

#include "renderer/context.h"

namespace renderer {

void Context::draw() {
  assert(vb_);
  assert(program_);

  auto transformed = invokeVertexShader(*vb_, *program_, uniform_);
  auto triangles = assembleTriangles(transformed);
  triangles = clipTriangles(triangles);
  convertToScreenSpace(triangles, fb_->getWidth(), fb_->getHeight());
  triangles = cullBackFacing(triangles);
  auto fragments = rasterize(triangles, program_->attr_count, wireframe_);
  invokeFragmentShader(fragments, *fb_, uniform_, program_->fs);
}

} // namespace renderer
