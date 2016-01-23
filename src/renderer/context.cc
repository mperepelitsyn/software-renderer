#include <cassert>

#include "renderer/context.h"
#include "renderer/arena.h"

namespace renderer {

void Context::draw() {
  assert(vb_);
  assert(program_);

  arena_.reset(vb_->count,
               program_->attr_count * sizeof(float) + sizeof(VertexH::pos));

  auto transformed = invokeVertexShader(*vb_, *program_, uniform_, arena_);
  auto triangles = assembleTriangles(transformed);
  triangles = clipTriangles(triangles);
  convertToScreenSpace(triangles, fb_->getWidth(), fb_->getHeight());
  triangles = cullBackFacing(triangles);
  auto fragments = rasterize(triangles, program_->attr_count, wireframe_);
  invokeFragmentShader(fragments, *fb_, uniform_, program_->fs);
}

} // namespace renderer
