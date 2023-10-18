#ifndef GAMEDATA_HPP_
#define GAMEDATA_HPP_

#include <bitset>

enum class Input { LeftDown, LeftUp, RightDown, RightUp };
enum class State { Playing, LeftWin, RightWin };

struct GameData {
  State m_state{State::Playing};
  std::bitset<4> m_input; // [ left down, left up, right down, right up]
};

#endif