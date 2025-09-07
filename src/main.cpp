#include <SDL2/SDL.h>
#include <iostream>
#include <memory>
#include <vector>
#include <cmath>

#include "core/renderer.hpp"
#include "core/input.hpp"
#include "core/game.hpp"
#include "menu/main_menu.hpp"
#include "games/space_invaders/space_invaders.hpp"
#include "games/flappy_bird/flappy_bird.hpp"

constexpr int WINDOW_WIDTH = 800;
constexpr int WINDOW_HEIGHT = 600;
constexpr int TARGET_FPS = 180;
constexpr float TARGET_FRAME_TIME = 1000.0f / TARGET_FPS;

enum class AppState {
    Menu,
    InGame,
    Quitting
};

class GameManager {
private:
    std::unique_ptr<core::Renderer> renderer_;
    std::unique_ptr<core::InputManager> input_;
    std::unique_ptr<menu::MainMenu> main_menu_;
    std::vector<std::unique_ptr<games::Game> > games_;

    AppState app_state_{AppState::Menu};
    size_t current_game_index_{0};
    bool running_{true};
    bool escape_was_pressed_{false};

    static void initializeSDL() {
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) < 0) {
            throw std::runtime_error("Failed to initialize SDL: " + std::string(SDL_GetError()));
        }
    }

    void setupGames() {
        games_.push_back(std::make_unique<games::space_invaders::SpaceInvadersGame>());
        games_.push_back(std::make_unique<games::flappy_bird::FlappyBirdGame>());
    }

    void setupMenu() {
        main_menu_ = std::make_unique<menu::MainMenu>();

        for (size_t i = 0; i < games_.size(); ++i) {
            main_menu_->addItem(games_[i]->getName(), [this, i]() {
                current_game_index_ = i;
                games_[current_game_index_]->reset();
                app_state_ = AppState::InGame;
            });
        }

        main_menu_->addItem("Quit", [this]() {
            app_state_ = AppState::Quitting;
        });
    }

    void handleEvents() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                app_state_ = AppState::Quitting;
            }
        }

        input_->update();

        if (input_->isEscapePressed()) {
            if (!escape_was_pressed_) {
                if (app_state_ == AppState::InGame) {
                    app_state_ = AppState::Menu;
                } else if (app_state_ == AppState::Menu) {
                    app_state_ = AppState::Quitting;
                }
                escape_was_pressed_ = true;
            }
        } else {
            escape_was_pressed_ = false;
        }
    }

    void update(const float dt) {
        switch (app_state_) {
            case AppState::Menu:
                main_menu_->update(*input_);
                break;

            case AppState::InGame:
                if (current_game_index_ < games_.size()) {
                    games_[current_game_index_]->update(dt, *input_);

                    if (games_[current_game_index_]->getState() == games::GameState::GameOver) {
                    }
                }
                break;

            case AppState::Quitting:
                running_ = false;
                break;
        }
    }

    void render() const {
        switch (app_state_) {
            case AppState::Menu:
                main_menu_->render(*renderer_);
                break;

            case AppState::InGame:
                if (current_game_index_ < games_.size()) {
                    games_[current_game_index_]->render(*renderer_);
                }
                break;

            case AppState::Quitting:
                break;
        }

        renderer_->present();
    }

public:
    GameManager() {
        initializeSDL();

        renderer_ = std::make_unique<core::Renderer>("Retro Games Collection",
                                                     WINDOW_WIDTH, WINDOW_HEIGHT);
        input_ = std::make_unique<core::InputManager>();

        setupGames();
        setupMenu();

        std::cout << "Retro Games Collection initialized!\n";
        std::cout << "Controls:\n";
        std::cout << "  Menu: Arrow keys or D-pad to navigate, Space/Enter/A button to select\n";
        std::cout << "  Games: Arrow keys or left stick to move, Space/A button to shoot/jump\n";
        std::cout << "  ESC/Y Button: Return to menu or quit\n";

        if (input_->hasController()) {
            std::cout << "Controller detected and ready!\n";
        }
    }

    ~GameManager() {
        SDL_Quit();
    }

    GameManager(const GameManager &) = delete;

    GameManager &operator=(const GameManager &) = delete;

    void run() {
        Uint32 last_time = SDL_GetTicks();

        while (running_) {
            const Uint32 current_time = SDL_GetTicks();
            float delta_time = static_cast<float>(current_time - last_time) / 1000.0f;
            last_time = current_time;

            delta_time = std::min(delta_time, 1.0f / 30.0f);

            handleEvents();
            update(delta_time);
            render();

            if (const Uint32 frame_time = SDL_GetTicks() - current_time; static_cast<float>(frame_time) < TARGET_FRAME_TIME) {
                SDL_Delay(static_cast<Uint32>(TARGET_FRAME_TIME - static_cast<float>(frame_time)));
            }
        }
    }
};

int main([[maybe_unused]] int argc, [[maybe_unused]] char *argv[]) {
    try {
        GameManager manager;
        manager.run();
        return 0;
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
