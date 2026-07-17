#include <format>
#include <utility>

#include "app/app.h"

namespace app {

App::App(unsigned w, unsigned h, std::string name)
    : fb_{w, h}, width_{w}, height_{h}, name_{std::move(name)}, fps_counter_{name_, 0.25} {
  if (!SDL_Init(SDL_INIT_VIDEO))
    throw Error{std::format("failed to initialize SDL: {}", SDL_GetError())};

  if (!SDL_CreateWindowAndRenderer(name_.c_str(), w, h, 0, &window_, &renderer_))
    throw Error{std::format("failed to create SDL window: {}", SDL_GetError())};

  texture_ =
      SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, w, h);
  if (!texture_)
    throw Error{std::format("failed to create SDL texture: {}", SDL_GetError())};
  SDL_SetTextureScaleMode(texture_, SDL_SCALEMODE_NEAREST);

  ctx_.setFrameBuffer(&fb_);
  fps_counter_.setWindow(window_);
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

    SDL_UpdateTexture(texture_, nullptr, fb_.getColorTexture().getRawBuffer(), width_ * 4);
    // Framebuffer row 0 is the bottom scanline (GL convention); SDL draws row 0 at the top.
    SDL_RenderTextureRotated(renderer_, texture_, nullptr, nullptr, 0.0, nullptr,
                             SDL_FLIP_VERTICAL);
    SDL_RenderPresent(renderer_);
  }
  shutdown();
}

} // namespace app
