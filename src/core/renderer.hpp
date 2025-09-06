#pragma once
#include <SDL2/SDL.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <string_view>
#include <array>
#include <cmath>
#include <memory>
#include "text.hpp"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace core {

class Renderer {
private:
    SDL_Window* window_{};
    SDL_GLContext context_{};
    int width_{}, height_{};
    std::unique_ptr<FontManager> font_manager_;
    std::unique_ptr<TextRenderer> text_renderer_;
    
public:
    Renderer(std::string_view title, int width, int height) 
        : width_(width), height_(height) {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
        
        window_ = SDL_CreateWindow(title.data(), 
                                  SDL_WINDOWPOS_CENTERED, 
                                  SDL_WINDOWPOS_CENTERED, 
                                  width, height, 
                                  SDL_WINDOW_OPENGL);
        
        context_ = SDL_GL_CreateContext(window_);
        
        glViewport(0, 0, width, height);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluOrtho2D(0, width, 0, height);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        // Initialize text rendering
        font_manager_ = std::make_unique<FontManager>();
        text_renderer_ = std::make_unique<TextRenderer>(*font_manager_);
    }
    
    ~Renderer() {
        if (context_) SDL_GL_DeleteContext(context_);
        if (window_) SDL_DestroyWindow(window_);
    }
    
    // Delete copy constructor and assignment
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;
    
    void clear(float r = 0.0f, float g = 0.0f, float b = 0.0f) const noexcept {
        glClearColor(r, g, b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }
    
    void present() const noexcept {
        SDL_GL_SwapWindow(window_);
    }
    
    void setColor(float r, float g, float b, float a = 1.0f) const noexcept {
        glColor4f(r, g, b, a);
    }
    
    void drawRect(float x, float y, float w, float h) const noexcept {
        glBegin(GL_QUADS);
        glVertex2f(x - w/2, y - h/2);
        glVertex2f(x + w/2, y - h/2);
        glVertex2f(x + w/2, y + h/2);
        glVertex2f(x - w/2, y + h/2);
        glEnd();
    }
    
    void drawCircle(float x, float y, float radius, int segments = 32) const noexcept {
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(x, y);
        
        for (int i = 0; i <= segments; ++i) {
            float angle = 2.0f * M_PI * i / segments;
            glVertex2f(x + radius * std::cos(angle), y + radius * std::sin(angle));
        }
        glEnd();
    }
    
    [[nodiscard]] constexpr int getWidth() const noexcept { return width_; }
    [[nodiscard]] constexpr int getHeight() const noexcept { return height_; }
    
    // Text rendering methods
    void drawText(const std::string& text, float x, float y, 
                  float scale = 1.0f, const Color& color = Color{},
                  TextAlign align = TextAlign::Left) {
        text_renderer_->drawText(text, x, y, scale, color, align);
    }
    
    void drawTextCentered(const std::string& text, float center_x, float y,
                         float scale = 1.0f, const Color& color = Color{}) {
        text_renderer_->drawTextCentered(text, center_x, y, scale, color);
    }
    
    [[nodiscard]] float getTextWidth(const std::string& text, float scale = 1.0f) const {
        return text_renderer_->getTextWidth(text, scale);
    }
};

} // namespace core
