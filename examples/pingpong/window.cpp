#include "window.hpp"

void Window::onEvent(SDL_Event const &event) {
  // Keyboard events
  if (event.type == SDL_KEYDOWN) {
    if (event.key.keysym.sym == SDLK_w)
      m_gameData.m_input.set(gsl::narrow<size_t>(LeftInput::Up));
    if (event.key.keysym.sym == SDLK_s)
      m_gameData.m_input.set(gsl::narrow<size_t>(LeftInput::Down));
    if (event.key.keysym.sym == SDLK_UP)
      m_gameData.m_input.set(gsl::narrow<size_t>(RightInput::Up));
    if (event.key.keysym.sym == SDLK_DOWN)
      m_gameData.m_input.set(gsl::narrow<size_t>(RightInput::Down));
  }
  if (event.type == SDL_KEYUP) {
    if (event.key.keysym.sym == SDLK_w)
      m_gameData.m_input.reset(gsl::narrow<size_t>(LeftInput::Up));
    if (eevent.key.keysym.sym == SDLK_s)
      m_gameData.m_input.reset(gsl::narrow<size_t>(LeftInput::Down));
    if (event.key.keysym.sym == SDLK_UP)
      m_gameData.m_input.reset(gsl::narrow<size_t>(RightInput::Up));
    if (event.key.keysym.sym == SDLK_DOWN)
      m_gameData.m_input.reset(gsl::narrow<size_t>(RightInput::Down));
  }
}

void Window::onCreate() {
  auto const assetsPath{abcg::Application::getAssetsPath()};

  // Load a new font
  auto const filename{assetsPath + "Inconsolata-Medium.ttf"};
  m_font = ImGui::GetIO().Fonts->AddFontFromFileTTF(filename.c_str(), 60.0f);
  if (m_font == nullptr) {
    throw abcg::RuntimeError("Cannot load font file");
  }

  // Create program to render the other objects
  m_objectsProgram =
      abcg::createOpenGLProgram({{.source = assetsPath + "objects.vert",
                                  .stage = abcg::ShaderStage::Vertex},
                                 {.source = assetsPath + "objects.frag",
                                  .stage = abcg::ShaderStage::Fragment}});

  abcg::glClearColor(0, 0, 0, 1);

#if !defined(__EMSCRIPTEN__)
  abcg::glEnable(GL_PROGRAM_POINT_SIZE);
#endif

  // Start pseudo-random number generator
  m_randomEngine.seed(
      std::chrono::steady_clock::now().time_since_epoch().count());

  restart();
}

void Window::restart() {
  m_gameData.m_state = State::Playing;

  m_ship.create(m_objectsProgram);
  m_asteroids.create(m_objectsProgram, 3);
  m_bullets.create(m_objectsProgram);
}

void Window::onUpdate() {
  auto const deltaTime{gsl::narrow_cast<float>(getDeltaTime())};

  // Wait 5 seconds before restarting
  if (m_gameData.m_state != State::Playing &&
      m_restartWaitTimer.elapsed() > 5) {
    restart();
    return;
  }

  m_left.update(m_gameData, deltaTime);
  m_right.update(m_gameData, deltaTime);
  m_asteroids.update(m_ship, deltaTime);
  m_bullets.update(m_ship, m_gameData, deltaTime);

  if (m_gameData.m_state == State::Playing) {
    checkCollisions();
    checkWinCondition();
  }
}

void Window::onPaint() {
  abcg::glClear(GL_COLOR_BUFFER_BIT);
  abcg::glViewport(0, 0, m_viewportSize.x, m_viewportSize.y);

  m_asteroids.paint();
  m_bullets.paint();
  m_left.paint(m_gameData);
  m_right.paint(m_gameData);
}

void Window::onPaintUI() {
  abcg::OpenGLWindow::onPaintUI();

  {
    auto const size{ImVec2(300, 85)};
    auto const position{ImVec2((m_viewportSize.x - size.x) / 2.0f,
                               (m_viewportSize.y - size.y) / 2.0f)};
    ImGui::SetNextWindowPos(position);
    ImGui::SetNextWindowSize(size);
    ImGuiWindowFlags const flags{ImGuiWindowFlags_NoBackground |
                                 ImGuiWindowFlags_NoTitleBar |
                                 ImGuiWindowFlags_NoInputs};
    ImGui::Begin(" ", nullptr, flags);
    ImGui::PushFont(m_font);

    if (m_gameData.m_state == State::LeftWin) {
      ImGui::Text("Left Player Won!");
    } else if (m_gameData.m_state == State::RightWin) {
      ImGui::Text("*Right Player Won!*");
    }

    ImGui::PopFont();
    ImGui::End();
  }
}

void Window::onResize(glm::ivec2 const &size) {
  m_viewportSize = size;

  abcg::glClear(GL_COLOR_BUFFER_BIT);
}

void Window::onDestroy() {
  abcg::glDeleteProgram(m_objectsProgram);

  m_asteroids.destroy();
  m_bullets.destroy();
  m_left.destroy();
  m_right.destroy();
}

void Window::checkCollisions() {
  // Check collision between ship and asteroids
  for (auto const &asteroid : m_asteroids.m_asteroids) {
    auto const asteroidTranslation{asteroid.m_translation};
    auto const distance{
        glm::distance(m_ship.m_translation, asteroidTranslation)};

    if (distance < m_ship.m_scale * 0.9f + asteroid.m_scale * 0.85f) {
      m_gameData.m_state = State::GameOver;
      m_restartWaitTimer.restart();
    }
  }

  // Check collision between bullets and asteroids
  for (auto &bullet : m_bullets.m_bullets) {
    if (bullet.m_dead)
      continue;

    for (auto &asteroid : m_asteroids.m_asteroids) {
      for (auto const i : {-2, 0, 2}) {
        for (auto const j : {-2, 0, 2}) {
          auto const asteroidTranslation{asteroid.m_translation +
                                         glm::vec2(i, j)};
          auto const distance{
              glm::distance(bullet.m_translation, asteroidTranslation)};

          if (distance < m_bullets.m_scale + asteroid.m_scale * 0.85f) {
            asteroid.m_hit = true;
            bullet.m_dead = true;
          }
        }
      }
    }

    // Break asteroids marked as hit
    for (auto const &asteroid : m_asteroids.m_asteroids) {
      if (asteroid.m_hit && asteroid.m_scale > 0.10f) {
        std::uniform_real_distribution randomDist{-1.0f, 1.0f};
        std::generate_n(std::back_inserter(m_asteroids.m_asteroids), 3, [&]() {
          glm::vec2 const offset{randomDist(m_randomEngine),
                                 randomDist(m_randomEngine)};
          auto const newScale{asteroid.m_scale * 0.5f};
          return m_asteroids.makeAsteroid(
              asteroid.m_translation + offset * newScale, newScale);
        });
      }
    }

    m_asteroids.m_asteroids.remove_if([](auto const &a) { return a.m_hit; });
  }
}

void Window::checkWinCondition() {
  if (m_asteroids.m_asteroids.empty()) {
    m_gameData.m_state = State::Win;
    m_restartWaitTimer.restart();
  }
}