#include <format>
#include <string_view>

#include <font8x8_basic.h>

#include "app/app.h"

namespace app {

namespace {

// Draws 8x8 bitmap text at (x, y) in window coords (origin top-left); the
// framebuffer stores row 0 as the bottom scanline, hence the flip.
void drawText(renderer::FrameBuffer &fb, unsigned x, unsigned y, unsigned scale,
              std::string_view text) {
  const renderer::Vec4 white{1.f, 1.f, 1.f, 1.f};
  for (auto ch : text) {
    auto &glyph = font8x8_basic[static_cast<unsigned char>(ch) & 0x7f];
    for (auto dy = 0u; dy < 8 * scale; ++dy)
      for (auto dx = 0u; dx < 8 * scale; ++dx) {
        if (!(glyph[dy / scale] >> (dx / scale) & 1))
          continue;
        auto px = x + dx;
        auto py = y + dy;
        if (px < fb.getWidth() && py < fb.getHeight())
          fb.setPixel(px, fb.getHeight() - 1 - py, white, 0.f);
      }
    x += 8 * scale;
  }
}

} // namespace

App::App(unsigned w, unsigned h, const std::string &name)
    : fb_{w, h}, width_{w}, height_{h}, fps_counter_{0.25} {
  if (!SDL_Init(SDL_INIT_VIDEO))
    throw Error{std::format("failed to initialize SDL: {}", SDL_GetError())};

  if (!SDL_CreateWindowAndRenderer(name.c_str(), w, h, 0, &window_, &renderer_))
    throw Error{std::format("failed to create SDL window: {}", SDL_GetError())};

  texture_ =
      SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, w, h);
  if (!texture_)
    throw Error{std::format("failed to create SDL texture: {}", SDL_GetError())};
  SDL_SetTextureScaleMode(texture_, SDL_SCALEMODE_NEAREST);

  ctx_.setFrameBuffer(&fb_);
}

App::~App() {
  SDL_DestroyTexture(texture_);
  SDL_DestroyRenderer(renderer_);
  SDL_DestroyWindow(window_);
  SDL_Quit();
}

void App::render() {
  startup();
  bool running = true;
  while (running) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT ||
          (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE))
        running = false;
    }

    auto time = SDL_GetTicksNS() / 1e9;
    auto delta = time - last_time_;
    last_time_ = time;

    fps_counter_.tick(delta);
    renderLoop(time, delta);

    auto &stats = ctx_.getStats();
    drawText(fb_, 8, 8, 2,
             std::format("{:.0f} fps  {:.1f} ms  vtx {:.1f}  ras {:.1f}", fps_counter_.fps(),
                         fps_counter_.frameMs(), stats.vtx_ms, stats.raster_ms));
    drawText(fb_, 8, 28, 2,
             std::format("tris {}/{}  frag {:.2f}M", stats.drawn, stats.submitted,
                         static_cast<double>(stats.fragments) / 1e6));
    ctx_.resetStats();

    SDL_UpdateTexture(texture_, nullptr, fb_.getColorTexture().getRawBuffer(), width_ * 4);
    // Framebuffer row 0 is the bottom scanline (GL convention); SDL draws row 0 at the top.
    SDL_RenderTextureRotated(renderer_, texture_, nullptr, nullptr, 0.0, nullptr,
                             SDL_FLIP_VERTICAL);
    SDL_RenderPresent(renderer_);
  }
  shutdown();
}

} // namespace app
