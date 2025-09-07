#pragma once
#include <SDL2/SDL.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <string_view>
#include <cmath>
#include <memory>
#include "text.hpp"


namespace core {
    class Renderer {
        SDL_Window *window_{};
        SDL_GLContext context_{};
        int width_{}, height_{};
        std::unique_ptr<FontManager> font_manager_;
        std::unique_ptr<TextRenderer> text_renderer_;

    public:
        Renderer(const std::string_view title, const int width, const int height)
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
            font_manager_ = std::make_unique<FontManager>();
            text_renderer_ = std::make_unique<TextRenderer>(*font_manager_);
        }

        ~Renderer() {
            if (context_) SDL_GL_DeleteContext(context_);
            if (window_) SDL_DestroyWindow(window_);
        }

        Renderer(const Renderer &) = delete;

        Renderer &operator=(const Renderer &) = delete;

        static void clear(const float r = 0.0f, const float g = 0.0f, const float b = 0.0f) noexcept {
            glClearColor(r, g, b, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
        }

        void present() const noexcept {
            SDL_GL_SwapWindow(window_);
        }

        static void setColor(const float r, const float g, const float b, const float a = 1.0f) noexcept {
            glColor4f(r, g, b, a);
        }

        static void drawRect(const float x, const float y, const float w, const float h) noexcept {
            glBegin(GL_QUADS);
            glVertex2f(x - w / 2, y - h / 2);
            glVertex2f(x + w / 2, y - h / 2);
            glVertex2f(x + w / 2, y + h / 2);
            glVertex2f(x - w / 2, y + h / 2);
            glEnd();
        }

        static void drawCircle(const float x, const float y, const float radius, const int segments = 32) noexcept {
            glBegin(GL_TRIANGLE_FAN);
            glVertex2f(x, y);

            for (int i = 0; i <= segments; ++i) {
                const float angle = 2.0f * M_PI * i / segments;
                glVertex2f(x + radius * std::cos(angle), y + radius * std::sin(angle));
            }
            glEnd();
        }

        [[nodiscard]] constexpr int getWidth() const noexcept { return width_; }
        [[nodiscard]] constexpr int getHeight() const noexcept { return height_; }

        void drawText(const std::string &text, const float x, const float y,
                      const float scale = 1.0f, const Color &color = Color{},
                      const TextAlign align = TextAlign::Left) const {
            text_renderer_->drawText(text, x, y, scale, color, align);
        }

        void drawTextCentered(const std::string &text, const float center_x, const float y,
                              const float scale = 1.0f, const Color &color = Color{}) const {
            text_renderer_->drawTextCentered(text, center_x, y, scale, color);
        }

        [[nodiscard]] float getTextWidth(const std::string &text, const float scale = 1.0f) const {
            return text_renderer_->getTextWidth(text, scale);
        }
    };
}
