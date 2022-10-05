#ifndef WINDOW_HPP_
#define WINDOW_HPP_

#include "abcgOpenGL.hpp"

class Window : public abcg::OpenGLWindow {
protected:
  void onCreate() override;
  void onPaint() override;
  void onPaintUI() override;

private:
  std::array<float, 4> m_clearColor{0.108f, 0.203f, 0.316f, 1.0f};
};

#endif