#pragma once
#include "../../core/game.hpp"
#include "../../core/entity.hpp"
#include "../../core/math.hpp"
#include "../../core/input.hpp"
#include "../../core/renderer.hpp"
#include <vector>
#include <memory>
#include <random>
#include <algorithm>

namespace games::flappy_bird {
    class Bird final : public core::Entity {
    public:
        float velocity_y{0.0f};
        static constexpr float GRAVITY = -800.0f;
        static constexpr float JUMP_STRENGTH = 350.0f;

        Bird() : Entity({100.0f, 300.0f}, {20.0f, 20.0f}) {
        }

        void update(const float dt) override {
            velocity_y += GRAVITY * dt;
            pos.y += velocity_y * dt;

            if (pos.y < size.y / 2) {
                pos.y = size.y / 2;
                velocity_y = 0.0f;
            }

            if (pos.y > 600 - size.y / 2) {
                pos.y = 600 - size.y / 2;
                velocity_y = 0.0f;
            }
        }

        void render() override {
            // This will be handled by the game renderer
        }

        void jump() noexcept {
            velocity_y = JUMP_STRENGTH;
        }

        [[nodiscard]] bool isOnGround() const noexcept {
            return pos.y <= size.y / 2 + 1.0f;
        }
    };

    class Pipe final : public core::Entity {
    public:
        static constexpr float WIDTH = 60.0f;
        static constexpr float GAP_SIZE = 150.0f;
        float gap_center_y{};
        bool scored{false};

        Pipe(float x, const float gap_y)
            : Entity({x, 300.0f}, {WIDTH, 600.0f}), gap_center_y(gap_y) {
        }

        void update(const float dt) override {
            pos.x -= 150.0f * dt;

            if (pos.x < -size.x / 2) {
                active = false;
            }
        }

        void render() override {
            // This will be handled by the game renderer
        }

        [[nodiscard]] bool checkCollision(const Bird &bird) const noexcept {
            if (bird.pos.x + bird.size.x / 2 < pos.x - size.x / 2 ||
                bird.pos.x - bird.size.x / 2 > pos.x + size.x / 2) {
                return false;
            }

            const float gap_top = gap_center_y + GAP_SIZE / 2;
            const float gap_bottom = gap_center_y - GAP_SIZE / 2;

            return bird.pos.y + bird.size.y / 2 > gap_top ||
                   bird.pos.y - bird.size.y / 2 < gap_bottom;
        }

        [[nodiscard]] bool isPastBird(const Bird &bird) const noexcept {
            return !scored && pos.x + size.x / 2 < bird.pos.x - bird.size.x / 2;
        }
    };

    class FlappyBirdGame final : public Game {
        std::unique_ptr<Bird> bird_;
        std::vector<std::unique_ptr<Pipe> > pipes_;

        GameState state_{GameState::Playing};
        float pipe_spawn_timer_{0.0f};
        int score_{0};

        std::random_device rd_;
        std::mt19937 gen_{rd_()};
        std::uniform_real_distribution<float> gap_dist_{150.0f, 450.0f};

        void spawnPipe() {
            float gap_y = gap_dist_(gen_);
            auto pipe = std::make_unique<Pipe>(850.0f, gap_y);
            pipes_.push_back(std::move(pipe));
        }

        void checkCollisions() {
            for (const auto &pipe: pipes_) {
                if (pipe->checkCollision(*bird_)) {
                    state_ = GameState::GameOver;
                    return;
                }

                if (pipe->isPastBird(*bird_)) {
                    pipe->scored = true;
                    score_++;
                }
            }

            if (bird_->isOnGround() || bird_->pos.y >= 600 - bird_->size.y / 2) {
                state_ = GameState::GameOver;
            }
        }

        void cleanupPipes() {
            std::erase_if(pipes_,
                          [](const auto &pipe) { return !pipe->active; });
        }

    public:
        FlappyBirdGame() {
            reset();
        }

        void update(const float dt, core::InputManager &input) override {
            if (state_ == GameState::Playing) {
                if (input.isShootJustPressed()) {
                    bird_->jump();
                }

                bird_->update(dt);

                pipe_spawn_timer_ += dt;
                if (pipe_spawn_timer_ > 2.5f) {
                    spawnPipe();
                    pipe_spawn_timer_ = 0.0f;
                }

                for (const auto &pipe: pipes_) {
                    pipe->update(dt);
                }

                checkCollisions();
                cleanupPipes();
            } else if (state_ == GameState::GameOver) {
                if (input.isShootJustPressed()) {
                    reset();
                }
            }
        }

        void render(core::Renderer &renderer) override {
            core::Renderer::clear(0.5f, 0.8f, 1.0f);

            if (state_ == GameState::Playing || state_ == GameState::GameOver) {
                core::Renderer::setColor(0.0f, 0.8f, 0.0f);
                for (const auto &pipe: pipes_) {
                    if (pipe->active) {
                        const float top_height = (600.0f - pipe->gap_center_y - Pipe::GAP_SIZE / 2);
                        const float top_center_y = pipe->gap_center_y + Pipe::GAP_SIZE / 2 + top_height / 2;
                        core::Renderer::drawRect(pipe->pos.x, top_center_y, pipe->size.x, top_height);

                        const float bottom_height = pipe->gap_center_y - Pipe::GAP_SIZE / 2;
                        const float bottom_center_y = bottom_height / 2;
                        core::Renderer::drawRect(pipe->pos.x, bottom_center_y, pipe->size.x, bottom_height);
                    }
                }

                core::Renderer::setColor(1.0f, 1.0f, 0.0f);
                core::Renderer::drawCircle(bird_->pos.x, bird_->pos.y, bird_->size.x / 2, 16);

                renderer.drawText("SCORE: " + std::to_string(score_),
                                  20.0f, 580.0f, 1.5f,
                                  core::Color{1.0f, 1.0f, 1.0f});
            }

            if (state_ == GameState::GameOver) {
                core::Renderer::setColor(0.0f, 0.0f, 0.0f, 0.8f);
                core::Renderer::drawRect(400.0f, 300.0f, 500.0f, 200.0f);

                renderer.drawTextCentered("GAME OVER",
                                          400.0f, 360.0f, 2.5f,
                                          core::Color{1.0f, 0.3f, 0.3f});
                renderer.drawTextCentered("Score: " + std::to_string(score_),
                                          400.0f, 320.0f, 1.8f,
                                          core::Color{1.0f, 1.0f, 0.3f});
                renderer.drawTextCentered("Press Space or A to restart",
                                          400.0f, 280.0f, 1.2f,
                                          core::Color{0.9f, 0.9f, 0.9f});
                renderer.drawTextCentered("Press ESC to return to menu",
                                          400.0f, 250.0f, 1.0f,
                                          core::Color{0.7f, 0.7f, 0.7f});
            }
        }

        [[nodiscard]] GameState getState() const override {
            return state_;
        }

        void reset() override {
            state_ = GameState::Playing;
            score_ = 0;
            pipe_spawn_timer_ = 0.0f;

            bird_ = std::make_unique<Bird>();
            pipes_.clear();
        }

        [[nodiscard]] const char *getName() const override {
            return "Flappy Bird";
        }
    };
}
