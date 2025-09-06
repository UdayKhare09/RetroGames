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

class Player : public core::Entity {
public:
    core::Vector2 velocity{};
    float fire_cooldown{0.0f};
    
    Player(float screen_width) 
        : Entity({screen_width / 2.0f, 50.0f}, {20.0f, 20.0f}) {}
    
    void update(float dt) override {
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

class Invader : public core::Entity {
public:
    core::Vector2 velocity{};
    
    Invader(core::Vector2 position) 
        : Entity(position, {15.0f, 15.0f}), velocity({20.0f, 0.0f}) {}
    
    void update(float dt) override {
        pos += velocity * dt;
    }
    
    void render() override {
        // This will be handled by the game renderer
    }
};

class Bullet : public core::Entity {
public:
    core::Vector2 velocity{};
    bool is_player_bullet{true};
    
    Bullet(core::Vector2 position, core::Vector2 vel, bool player_bullet = true)
        : Entity(position, {2.0f, 5.0f}), velocity(vel), is_player_bullet(player_bullet) {}
    
    void update(float dt) override {
        pos += velocity * dt;
        
        // Deactivate if off screen
        if (pos.y < 0 || pos.y > 600) {
            active = false;
        }
    }
    
    void render() override {
        // This will be handled by the game renderer
    }
};

class SpaceInvadersGame : public Game {
private:
    std::unique_ptr<Player> player_;
    std::vector<std::unique_ptr<Invader>> invaders_;
    std::vector<std::unique_ptr<Bullet>> bullets_;
    
    GameState state_{GameState::Playing};
    float invader_move_timer_{0.0f};
    int invader_direction_{1};
    int score_{0};
    
    void createInvaders() {
        invaders_.clear();
        for (int row = 0; row < 5; ++row) {
            for (int col = 0; col < 10; ++col) {
                auto invader = std::make_unique<Invader>(
                    core::Vector2{50.0f + col * 60.0f, 600.0f - 100.0f - row * 30.0f}
                );
                invaders_.push_back(std::move(invader));
            }
        }
    }
    
    void updateInvaders(float dt) {
        invader_move_timer_ += dt;
        
        if (invader_move_timer_ > 1.0f) {
            invader_move_timer_ = 0.0f;
            
            // Move invaders
            for (auto& invader : invaders_) {
                if (invader->active) {
                    invader->pos.x += invader->velocity.x;
                }
            }
            
            // Check if direction change needed
            bool change_direction = false;
            for (const auto& invader : invaders_) {
                if (invader->active && (invader->pos.x < 20 || invader->pos.x > 780)) {
                    change_direction = true;
                    break;
                }
            }
            
            if (change_direction) {
                invader_direction_ *= -1;
                for (auto& invader : invaders_) {
                    if (invader->active) {
                        invader->velocity.x = 20.0f * invader_direction_;
                        invader->pos.y -= 10.0f;
                        
                        // Check if invaders reached player
                        if (invader->pos.y <= 70.0f) {
                            state_ = GameState::GameOver;
                        }
                    }
                }
            }
        }
    }
    
    void checkCollisions() {
        // Player bullets vs invaders
        for (auto& bullet : bullets_) {
            if (!bullet->active || !bullet->is_player_bullet) continue;
            
            for (auto& invader : invaders_) {
                if (!invader->active) continue;
                
                if (bullet->collidesWith(*invader)) {
                    bullet->active = false;
                    invader->active = false;
                    score_ += 10;
                    break;
                }
            }
        }
        
        // Check if all invaders destroyed
        bool any_active = std::any_of(invaders_.begin(), invaders_.end(),
                                     [](const auto& inv) { return inv->active; });
        if (!any_active) {
            createInvaders(); // Next wave
        }
    }
    
    void cleanupEntities() {
        bullets_.erase(
            std::remove_if(bullets_.begin(), bullets_.end(),
                          [](const auto& bullet) { return !bullet->active; }),
            bullets_.end()
        );
        
        invaders_.erase(
            std::remove_if(invaders_.begin(), invaders_.end(),
                          [](const auto& invader) { return !invader->active; }),
            invaders_.end()
        );
    }
    
public:
    SpaceInvadersGame() {
        reset();
    }
    
    void update(float dt, core::InputManager& input) override {
        if (state_ != GameState::Playing) return;
        
        // Update player movement
        player_->velocity.x = input.getHorizontalAxis() * 200.0f;
        player_->update(dt);
        
        // Handle shooting
        if (input.isShootJustPressed() && player_->canFire()) {
            auto bullet = std::make_unique<Bullet>(
                player_->pos + core::Vector2{0.0f, player_->size.y / 2.0f},
                core::Vector2{0.0f, 300.0f}
            );
            bullets_.push_back(std::move(bullet));
            player_->fired();
        }
        
        // Update entities
        for (auto& bullet : bullets_) {
            bullet->update(dt);
        }
        
        for (auto& invader : invaders_) {
            invader->update(dt);
        }
        
        updateInvaders(dt);
        checkCollisions();
        cleanupEntities();
    }
    
    void render(core::Renderer& renderer) override {
        renderer.clear(0.0f, 0.0f, 0.1f);
        
        if (state_ == GameState::Playing) {
            // Render player
            renderer.setColor(0.0f, 1.0f, 0.0f);
            renderer.drawRect(player_->pos.x, player_->pos.y, player_->size.x, player_->size.y);
            
            // Render invaders
            renderer.setColor(1.0f, 0.0f, 0.0f);
            for (const auto& invader : invaders_) {
                if (invader->active) {
                    renderer.drawRect(invader->pos.x, invader->pos.y, invader->size.x, invader->size.y);
                }
            }
            
            // Render bullets
            renderer.setColor(1.0f, 1.0f, 1.0f);
            for (const auto& bullet : bullets_) {
                if (bullet->active) {
                    renderer.drawRect(bullet->pos.x, bullet->pos.y, bullet->size.x, bullet->size.y);
                }
            }
            
            // Score display
            renderer.drawText("SCORE: " + std::to_string(score_), 
                            20.0f, 580.0f, 1.2f, 
                            core::Color{1.0f, 1.0f, 0.0f});
                            
            // Lives or other UI elements could go here
        } else if (state_ == GameState::GameOver) {
            // Game over screen
            renderer.setColor(0.0f, 0.0f, 0.0f, 0.7f);
            renderer.drawRect(400.0f, 300.0f, 600.0f, 200.0f);
            
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
    
    GameState getState() const override {
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
    
    const char* getName() const override {
        return "Space Invaders";
    }
};

} // namespace games::space_invaders
