#pragma once

namespace app {

class FPSCounter {
public:
  explicit FPSCounter(double update_freq) : freq_{update_freq} {}

  void tick(double delta) {
    ++frames_;
    elapsed_ += delta;
    if (elapsed_ >= freq_) {
      fps_ = frames_ / elapsed_;
      frames_ = 0;
      elapsed_ = 0.0;
    }
  }

  [[nodiscard]] double fps() const { return fps_; }
  [[nodiscard]] double frameMs() const { return fps_ > 0.0 ? 1000.0 / fps_ : 0.0; }

private:
  double freq_;
  double elapsed_{};
  double fps_{};
  unsigned frames_{};
};

} // namespace app
