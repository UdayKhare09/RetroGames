#pragma once
#include <memory>

namespace core {
class Renderer;
class InputManager;
}

namespace games {

enum class GameState {
    Menu,
    Playing,
    Paused,
    GameOver
};

class Game {
public:
    virtual ~Game() = default;
    virtual void update(float dt, core::InputManager& input) = 0;
    virtual void render(core::Renderer& renderer) = 0;
    virtual GameState getState() const = 0;
    virtual void reset() = 0;
    virtual const char* getName() const = 0;
};

} // namespace games
