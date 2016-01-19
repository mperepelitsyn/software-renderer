#include "renderer/context.h"

namespace renderer {

void Context::draw() {
  auto transformed = invokeVertexShader(vertices_, uniform_, vs_);
  auto triangles = assembleTriangles(transformed);
  triangles = clipTriangles(triangles);
  convertToScreenSpace(triangles, fb_->getWidth(), fb_->getHeight());
  triangles = cullBackFacing(triangles);
  auto fragments = rasterize(triangles, wireframe_);
  invokeFragmentShader(fragments, *fb_, uniform_, fs_);
}

} // namespace renderer
