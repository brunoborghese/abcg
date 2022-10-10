#ifndef WINDOW_HPP_
#define WINDOW_HPP_

#include <random>

#include "abcgOpenGL.hpp"

class Window : public abcg::OpenGLWindow {
protected:
  void onCreate() override;
  void onPaintUI() override;

private:
  static int const m_N{10};     // Board size is m_N x m_N
  static int const m_NBombs{1}; // Number of Bombs

  enum class GameState { Play, Win, Lose };
  GameState m_gameState;

  int movesLeft{};
  std::array<char, m_N * m_N> m_board{}; // '\0', 'X' or near bomb qty
  std::array<char, m_N * m_N> m_bombs{}; // bombs positions

  std::default_random_engine m_randomEngine;
  ImFont *m_font{};

  int clamp(int v, int min, int max);
  bool checkForBomb(int i, int j);
  int checkNeighbors(int i, int j);
  void checkEndCondition(bool lost);
  void restartGame();
};

#endif