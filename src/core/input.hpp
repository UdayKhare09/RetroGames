#pragma once
#include <SDL2/SDL.h>
#include <memory>
#include <cmath>

namespace core {

class InputManager {
private:
    const Uint8* keyboard_state_{};
    SDL_GameController* controller_{};
    bool prev_shoot_pressed_{false};
    bool curr_shoot_pressed_{false};
    
public:
    InputManager() {
        keyboard_state_ = SDL_GetKeyboardState(nullptr);
        
        // Open first available controller
        for (int i = 0; i < SDL_NumJoysticks(); ++i) {
            if (SDL_IsGameController(i)) {
                controller_ = SDL_GameControllerOpen(i);
                break;
            }
        }
    }
    
    ~InputManager() {
        if (controller_) {
            SDL_GameControllerClose(controller_);
        }
    }
    
    // Delete copy constructor and assignment
    InputManager(const InputManager&) = delete;
    InputManager& operator=(const InputManager&) = delete;
    
    void update() {
        prev_shoot_pressed_ = curr_shoot_pressed_;
        
        curr_shoot_pressed_ = keyboard_state_[SDL_SCANCODE_SPACE] || 
                             keyboard_state_[SDL_SCANCODE_UP] ||
                             keyboard_state_[SDL_SCANCODE_RETURN] ||
                             (controller_ && SDL_GameControllerGetButton(controller_, SDL_CONTROLLER_BUTTON_A));
    }
    
    [[nodiscard]] bool isKeyPressed(SDL_Scancode key) const noexcept {
        return keyboard_state_[key];
    }
    
    [[nodiscard]] bool isShootJustPressed() const noexcept {
        return curr_shoot_pressed_ && !prev_shoot_pressed_;
    }
    
    [[nodiscard]] bool isShootPressed() const noexcept {
        return curr_shoot_pressed_;
    }
    
    [[nodiscard]] float getHorizontalAxis() const noexcept {
        float axis = 0.0f;
        
        if (keyboard_state_[SDL_SCANCODE_LEFT] || keyboard_state_[SDL_SCANCODE_A]) {
            axis -= 1.0f;
        }
        if (keyboard_state_[SDL_SCANCODE_RIGHT] || keyboard_state_[SDL_SCANCODE_D]) {
            axis += 1.0f;
        }
        
        if (controller_) {
            Sint16 controller_axis = SDL_GameControllerGetAxis(controller_, SDL_CONTROLLER_AXIS_LEFTX);
            if (std::abs(controller_axis) > 8000) {
                axis = controller_axis / 32767.0f;
            }
        }
        
        return axis;
    }
    
    [[nodiscard]] bool isUpPressed() const noexcept {
        return keyboard_state_[SDL_SCANCODE_UP] || keyboard_state_[SDL_SCANCODE_W] ||
               (controller_ && SDL_GameControllerGetButton(controller_, SDL_CONTROLLER_BUTTON_DPAD_UP));
    }
    
    [[nodiscard]] bool isDownPressed() const noexcept {
        return keyboard_state_[SDL_SCANCODE_DOWN] || keyboard_state_[SDL_SCANCODE_S] ||
               (controller_ && SDL_GameControllerGetButton(controller_, SDL_CONTROLLER_BUTTON_DPAD_DOWN));
    }
    
    [[nodiscard]] bool isEscapePressed() const noexcept {
        return keyboard_state_[SDL_SCANCODE_ESCAPE] ||
               (controller_ && SDL_GameControllerGetButton(controller_, SDL_CONTROLLER_BUTTON_Y));  // Y/Triangle button
    }
    
    [[nodiscard]] bool hasController() const noexcept {
        return controller_ != nullptr;
    }
};

} // namespace core
