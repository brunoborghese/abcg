#include "window.hpp"

void Window::onCreate() {
  // Load font with bigger size for the X's and O's
  auto const filename{abcg::Application::getAssetsPath() +
                      "Inconsolata-Medium.ttf"};
  m_font = ImGui::GetIO().Fonts->AddFontFromFileTTF(filename.c_str(), 24.0f);
  if (m_font == nullptr) {
    throw abcg::RuntimeError{"Cannot load font file"};
  }

  // Start pseudorandom number generator
  auto const seed{std::chrono::steady_clock::now().time_since_epoch().count()};
  m_randomEngine.seed(seed);

  restartGame();
}

void Window::onPaintUI() {
  // Get size of application's window
  auto const appWindowWidth{gsl::narrow<float>(getWindowSettings().width)};
  auto const appWindowHeight{gsl::narrow<float>(getWindowSettings().height)};

  // "MineField" window
  {
    ImGui::SetNextWindowSize(ImVec2(appWindowWidth, appWindowHeight));
    ImGui::SetNextWindowPos(ImVec2(0, 0));

    auto const flags{ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoCollapse};
    ImGui::Begin("MineField", nullptr, flags);

    // Menu
    {
      bool restartSelected{};
      if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("Game")) {
          ImGui::MenuItem("Restart", nullptr, &restartSelected);
          ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
      }
      if (restartSelected) {
        restartGame();
      }
    }

    // Static text showing current turn or win/draw messages
    {
      std::string text;
      switch (m_gameState) {
      case GameState::Play:
        text = "Select Field";
        break;
      case GameState::Win:
        text = "You Won!";
        break;
      case GameState::Lose:
        text = "You Lost!";
        break;
      }
      // Center text
      ImGui::SetCursorPosX(
          (appWindowWidth - ImGui::CalcTextSize(text.c_str()).x) / 2);
      ImGui::Text("%s", text.c_str());
      ImGui::Spacing();
    }

    ImGui::Spacing();

    // Game board
    {
      auto const gridHeight{appWindowHeight - 22 - 60 - (m_N * 10) - 60};
      auto const buttonHeight{gridHeight / m_N};

      // Use custom font
      ImGui::PushFont(m_font);
      if (ImGui::BeginTable("Game board", m_N)) {
        for (auto i : iter::range(m_N)) {
          ImGui::TableNextRow();
          for (auto j : iter::range(m_N)) {
            ImGui::TableSetColumnIndex(j);
            auto const offset{i * m_N + j};

            // Get current character
            auto ch{m_board.at(offset)};

            // Replace null character with whitespace because the button label
            // cannot be an empty string
            if (ch == 0) {
              ch = ' ';
            }

            auto buttonColor{3 / 7.0f};
            if (ch == ' ') {
              buttonColor = 4 / 7.0f;
            }
            if (ch == 'X') {
              buttonColor = 0.0f;
            }

            // Button text is ch followed by an ID in the format ##ij
            auto buttonText{fmt::format("{}##{}{}", ch, i, j)};
            ImGui::PushID(i);
            ImGui::PushStyleColor(
                ImGuiCol_Button, (ImVec4)ImColor::HSV(buttonColor, 0.6f, 0.6f));

            ImGui::PushStyleColor(
                ImGuiCol_ButtonHovered,
                (ImVec4)ImColor::HSV(buttonColor, 0.7f, 0.7f));
            ImGui::PushStyleColor(
                ImGuiCol_ButtonActive,
                (ImVec4)ImColor::HSV(buttonColor, 0.8f, 0.8f));
            if (ImGui::Button(buttonText.c_str(), ImVec2(-1, buttonHeight))) {
              if (m_gameState == GameState::Play && ch == ' ') {
                bool isBomb = checkForBomb(i, j);
                auto bombsQty = checkNeighbors(i, j);
                auto aa = fmt::format("{}", bombsQty);
                m_board.at(offset) = isBomb ? 'X' : aa.at(0);
                checkEndCondition(isBomb);
              }
            }
            ImGui::PopStyleColor(3);
            ImGui::PopID();
          }
          ImGui::Spacing();
        }
        ImGui::EndTable();
      }
      ImGui::PopFont();
    }

    ImGui::Spacing();

    // "Restart game" button
    {
      if (ImGui::Button("Restart game", ImVec2(-1, 50))) {
        restartGame();
      }
    }

    ImGui::End();
  }
}

int Window::clampP(int v) { return std::clamp(v, 0, m_N - 1); }

bool Window::checkForBomb(int i, int j) {
  for (int ix = 0; ix < m_NBombs; ix++) {
    auto const offset{i * m_N + j};
    if (m_bombs.at(offset) == 'b') {
      return true;
    }
  }
  return false;
}

int Window::checkNeighbors(int i, int j) {
  int bombsQty = 0;
  if (checkForBomb(clampP(i - 1), clampP(j - 1)))
    bombsQty++;
  if (checkForBomb(clampP(i - 1), clampP(j)))
    bombsQty++;
  if (checkForBomb(clampP(i - 1), clampP(j + 1)))
    bombsQty++;
  if (checkForBomb(clampP(i), clampP(j - 1)))
    bombsQty++;
  if (checkForBomb(clampP(i), clampP(j + 1)))
    bombsQty++;
  if (checkForBomb(clampP(i + 1), clampP(j - 1)))
    bombsQty++;
  if (checkForBomb(clampP(i + 1), clampP(j)))
    bombsQty++;
  if (checkForBomb(clampP(i + 1), clampP(j + 1)))
    bombsQty++;
  return bombsQty;
}

void Window::checkEndCondition(bool lost) {
  if (m_gameState != GameState::Play) {
    return;
  }

  movesLeft = movesLeft - 1;

  if (lost) {
    m_gameState = GameState::Lose;
  }

  if (movesLeft == 0) {
    m_gameState = GameState::Win;
  }
}

void Window::restartGame() {
  m_board.fill('\0');
  m_bombs.fill(' ');
  std::uniform_int_distribution<int> intDistribution(0, m_N - 1);
  for (int ix = 0; ix < m_NBombs; ix++) {
    int i = intDistribution(m_randomEngine);
    int j = intDistribution(m_randomEngine);
    auto const offset{i * m_N + j};
    if (m_bombs.at(offset) == 'b') {
      ix--;
    } else {
      m_bombs.at(offset) = 'b';
    }
  }
  movesLeft = (m_N * m_N) - m_NBombs;
  m_gameState = GameState::Play;
}