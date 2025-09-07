#pragma once

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

        virtual void update(float dt, core::InputManager &input) = 0;

        virtual void render(core::Renderer &renderer) = 0;

        [[nodiscard]] virtual GameState getState() const = 0;

        virtual void reset() = 0;

        [[nodiscard]] virtual const char *getName() const = 0;
    };
}