#include "context.h"
#include "pipeline.h"

void Context::draw() {
  auto transformed = invokeVertexShader(*vertices_, uniform_, vs_);
  auto triangles = assembleTriangles(transformed);
  triangles = clipTriangles(triangles);
  triangles = cullBackFacing(triangles);
  convertToScreenSpace(triangles, fb_->getWidth(), fb_->getHeight());
  auto fragments = rasterize(triangles, wireframe_);
  invokeFragmentShader(fragments, *fb_, uniform_, fs_);
}
