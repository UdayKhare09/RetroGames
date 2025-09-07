#pragma once
#include "../../core/game.hpp"
#include "../../core/entity.hpp"
#include "../../core/math.hpp"
#include "../../core/input.hpp"
#include "../../core/renderer.hpp"
#include <vector>
#include <memory>
#include <algorithm>

namespace games::space_invaders {
    class Player final : public core::Entity {
    public:
        core::Vector2 velocity{};
        float fire_cooldown{0.0f};

        explicit Player(const float screen_width)
            : Entity({screen_width / 2.0f, 50.0f}, {20.0f, 20.0f}) {
        }

        void update(const float dt) override {
            pos += velocity * dt;

            // Keep player on screen
            if (pos.x < size.x / 2) pos.x = size.x / 2;
            if (pos.x > 800 - size.x / 2) pos.x = 800 - size.x / 2;

            fire_cooldown = std::max(0.0f, fire_cooldown - dt);
        }

        void render() override {
            // This will be handled by the game renderer
        }

        [[nodiscard]] bool canFire() const noexcept {
            return fire_cooldown <= 0.0f;
        }

        void fired() noexcept {
            fire_cooldown = 0.2f;
        }
    };

    class Invader final : public core::Entity {
    public:
        core::Vector2 velocity{};

        explicit Invader(const core::Vector2 position)
            : Entity(position, {15.0f, 15.0f}), velocity({20.0f, 0.0f}) {
        }

        void update(const float dt) override {
            pos += velocity * dt;
        }

        void render() override {
            // This will be handled by the game renderer
        }
    };

    class Bullet final : public core::Entity {
    public:
        core::Vector2 velocity{};
        bool is_player_bullet{true};

        Bullet(const core::Vector2 position, const core::Vector2 vel, const bool player_bullet = true)
            : Entity(position, {2.0f, 5.0f}), velocity(vel), is_player_bullet(player_bullet) {
        }

        void update(const float dt) override {
            pos += velocity * dt;

            if (pos.y < 0 || pos.y > 600) {
                active = false;
            }
        }

        void render() override {
            // This will be handled by the game renderer
        }
    };

    class SpaceInvadersGame final : public Game {
        std::unique_ptr<Player> player_;
        std::vector<std::unique_ptr<Invader> > invaders_;
        std::vector<std::unique_ptr<Bullet> > bullets_;

        GameState state_{GameState::Playing};
        float invader_move_timer_{0.0f};
        int invader_direction_{1};
        int score_{0};

        void createInvaders() {
            invaders_.clear();
            for (int row = 0; row < 5; ++row) {
                for (int col = 0; col < 10; ++col) {
                    auto invader = std::make_unique<Invader>(
                        core::Vector2{50.0f + static_cast<float>(col) * 60.0f, 600.0f - 100.0f - static_cast<float>(row) * 30.0f}
                    );
                    invaders_.push_back(std::move(invader));
                }
            }
        }

        void updateInvaders(const float dt) {
            invader_move_timer_ += dt;

            if (invader_move_timer_ > 1.0f) {
                invader_move_timer_ = 0.0f;

                for (const auto &invader: invaders_) {
                    if (invader->active) {
                        invader->pos.x += invader->velocity.x;
                    }
                }

                bool change_direction = false;
                for (const auto &invader: invaders_) {
                    if (invader->active && (invader->pos.x < 20 || invader->pos.x > 780)) {
                        change_direction = true;
                        break;
                    }
                }

                if (change_direction) {
                    invader_direction_ *= -1;
                    for (const auto &invader: invaders_) {
                        if (invader->active) {
                            invader->velocity.x = 20.0f * static_cast<float>(invader_direction_);
                            invader->pos.y -= 10.0f;

                            if (invader->pos.y <= 70.0f) {
                                state_ = GameState::GameOver;
                            }
                        }
                    }
                }
            }
        }

        void checkCollisions() {
            for (const auto &bullet: bullets_) {
                if (!bullet->active || !bullet->is_player_bullet) continue;

                for (auto &invader: invaders_) {
                    if (!invader->active) continue;

                    if (bullet->collidesWith(*invader)) {
                        bullet->active = false;
                        invader->active = false;
                        score_ += 10;
                        break;
                    }
                }
            }

            const bool any_active = std::ranges::any_of(invaders_,
                                                        [](const auto &inv) { return inv->active; });
            if (!any_active) {
                createInvaders();
            }
        }

        void cleanupEntities() {
            std::erase_if(bullets_,
                          [](const auto &bullet) { return !bullet->active; });

            std::erase_if(invaders_,
                          [](const auto &invader) { return !invader->active; });
        }

    public:
        SpaceInvadersGame() {
            reset();
        }

        void update(const float dt, core::InputManager &input) override {
            if (state_ != GameState::Playing) return;

            player_->velocity.x = input.getHorizontalAxis() * 200.0f;
            player_->update(dt);

            if (input.isShootJustPressed() && player_->canFire()) {
                auto bullet = std::make_unique<Bullet>(
                    player_->pos + core::Vector2{0.0f, player_->size.y / 2.0f},
                    core::Vector2{0.0f, 300.0f}
                );
                bullets_.push_back(std::move(bullet));
                player_->fired();
            }

            for (const auto &bullet: bullets_) {
                bullet->update(dt);
            }

            for (const auto &invader: invaders_) {
                invader->update(dt);
            }

            updateInvaders(dt);
            checkCollisions();
            cleanupEntities();
        }

        void render(core::Renderer &renderer) override {
            core::Renderer::clear(0.0f, 0.0f, 0.1f);

            if (state_ == GameState::Playing) {
                core::Renderer::setColor(0.0f, 1.0f, 0.0f);
                core::Renderer::drawRect(player_->pos.x, player_->pos.y, player_->size.x, player_->size.y);

                core::Renderer::setColor(1.0f, 0.0f, 0.0f);
                for (const auto &invader: invaders_) {
                    if (invader->active) {
                        core::Renderer::drawRect(invader->pos.x, invader->pos.y, invader->size.x, invader->size.y);
                    }
                }

                core::Renderer::setColor(1.0f, 1.0f, 1.0f);
                for (const auto &bullet: bullets_) {
                    if (bullet->active) {
                        core::Renderer::drawRect(bullet->pos.x, bullet->pos.y, bullet->size.x, bullet->size.y);
                    }
                }

                renderer.drawText("SCORE: " + std::to_string(score_),
                                  20.0f, 580.0f, 1.2f,
                                  core::Color{1.0f, 1.0f, 0.0f});
            } else if (state_ == GameState::GameOver) {
                core::Renderer::setColor(0.0f, 0.0f, 0.0f, 0.7f);
                core::Renderer::drawRect(400.0f, 300.0f, 600.0f, 200.0f);

                renderer.drawTextCentered("GAME OVER",
                                          400.0f, 350.0f, 2.5f,
                                          core::Color{1.0f, 0.2f, 0.2f});
                renderer.drawTextCentered("Final Score: " + std::to_string(score_),
                                          400.0f, 310.0f, 1.5f,
                                          core::Color{1.0f, 1.0f, 1.0f});
                renderer.drawTextCentered("Press ESC to return to menu",
                                          400.0f, 270.0f, 1.2f,
                                          core::Color{0.8f, 0.8f, 0.8f});
            }
        }

        [[nodiscard]] GameState getState() const override {
            return state_;
        }

        void reset() override {
            state_ = GameState::Playing;
            score_ = 0;
            invader_move_timer_ = 0.0f;
            invader_direction_ = 1;

            player_ = std::make_unique<Player>(800.0f);
            bullets_.clear();
            createInvaders();
        }

        [[nodiscard]] const char *getName() const override {
            return "Space Invaders";
        }
    };
}
