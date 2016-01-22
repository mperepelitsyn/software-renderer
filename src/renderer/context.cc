#include <cassert>

#include "renderer/context.h"

namespace renderer {

void Context::draw() {
  assert(vertices_);

  auto transformed = invokeVertexShader(vertices_, uniform_, vs_);
  auto triangles = assembleTriangles(transformed);
  triangles = clipTriangles(triangles);
  convertToScreenSpace(triangles, fb_->getWidth(), fb_->getHeight());
  triangles = cullBackFacing(triangles);
  auto fragments = rasterize(triangles, attr_size_, wireframe_);
  invokeFragmentShader(fragments, *fb_, uniform_, fs_);
}

} // namespace renderer
