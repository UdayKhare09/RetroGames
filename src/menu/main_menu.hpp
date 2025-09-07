#pragma once
#include "../core/input.hpp"
#include "../core/renderer.hpp"
#include <vector>
#include <string>
#include <functional>
#include <memory>

namespace menu {
    class MenuItem {
    public:
        std::string text;
        std::function<void()> action;

        MenuItem(std::string text, std::function<void()> action)
            : text(std::move(text)), action(std::move(action)) {
        }
    };

    class MainMenu {
    private:
        std::vector<MenuItem> items_;
        size_t selected_index_{0};
        bool prev_up_{false}, prev_down_{false};

    public:
        void addItem(const std::string &text, std::function<void()> action) {
            items_.emplace_back(text, std::move(action));
        }

        void update(const core::InputManager &input) {
            const bool up_pressed = input.isUpPressed();
            const bool down_pressed = input.isDownPressed();

            if (up_pressed && !prev_up_ && !items_.empty()) {
                selected_index_ = (selected_index_ == 0) ? items_.size() - 1 : selected_index_ - 1;
            }

            if (down_pressed && !prev_down_ && !items_.empty()) {
                selected_index_ = (selected_index_ + 1) % items_.size();
            }

            if (input.isShootJustPressed() && !items_.empty()) {
                items_[selected_index_].action();
            }

            prev_up_ = up_pressed;
            prev_down_ = down_pressed;
        }

        void render(core::Renderer &renderer) const {
            renderer.clear(0.05f, 0.05f, 0.1f);

            renderer.drawTextCentered("RETRO GAMES COLLECTION",
                                      renderer.getWidth() / 2.0f,
                                      renderer.getHeight() * 0.85f,
                                      2.0f,
                                      core::Color{1.0f, 1.0f, 0.2f});

            const float start_y = renderer.getHeight() * 0.6f;

            for (size_t i = 0; i < items_.size(); ++i) {
                constexpr float item_height = 60.0f;
                const float y = start_y - i * item_height;

                if (i == selected_index_) {
                    renderer.drawTextCentered(items_[i].text,
                                              renderer.getWidth() / 2.0f,
                                              y + 5.0f,
                                              1.5f,
                                              core::Color{1.0f, 1.0f, 1.0f});
                } else {
                    renderer.drawTextCentered(items_[i].text,
                                              renderer.getWidth() / 2.0f,
                                              y + 5.0f,
                                              1.2f,
                                              core::Color{0.8f, 0.8f, 0.8f});
                }
            }

            renderer.drawTextCentered("Use Arrow Keys or D-Pad to navigate",
                                      renderer.getWidth() / 2.0f,
                                      120.0f,
                                      1.0f,
                                      core::Color{0.6f, 0.6f, 0.6f});
            renderer.drawTextCentered("Press Space, Enter, or A Button to select",
                                      renderer.getWidth() / 2.0f,
                                      90.0f,
                                      1.0f,
                                      core::Color{0.6f, 0.6f, 0.6f});
            renderer.drawTextCentered("Press ESC to quit",
                                      renderer.getWidth() / 2.0f,
                                      60.0f,
                                      1.0f,
                                      core::Color{0.6f, 0.6f, 0.6f});
        }

        [[nodiscard]] size_t getSelectedIndex() const noexcept {
            return selected_index_;
        }

        void clear() {
            items_.clear();
            selected_index_ = 0;
        }
    };
}
