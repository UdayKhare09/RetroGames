#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <GL/gl.h>
#include <string>
#include <unordered_map>
#include <cmath>
#include <algorithm>
#include <ranges>

namespace core {
    struct Color {
        float r, g, b, a;

        constexpr explicit Color(float r = 1.0f, float g = 1.0f, float b = 1.0f, float a = 1.0f)
            : r(r), g(g), b(b), a(a) {
        }
    };

    enum class TextAlign {
        Left,
        Center,
        Right
    };

    class FontManager {
        std::unordered_map<std::string, TTF_Font *> fonts_;
        bool ttf_initialized_{false};
        TTF_Font *default_font_{nullptr};

    public:
        FontManager() {
            if (TTF_Init() == 0) {
                ttf_initialized_ = true;

                const char *font_paths[] = {
                    "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
                    "/usr/share/fonts/TTF/DejaVuSans-Bold.ttf",
                    "/System/Library/Fonts/Arial.ttf",
                    "/Windows/Fonts/arial.ttf",
                    "/usr/share/fonts/truetype/liberation/LiberationSans-Bold.ttf"
                };

                for (const char *path: font_paths) {
                    default_font_ = TTF_OpenFont(path, 24);
                    if (default_font_) {
                        TTF_SetFontHinting(default_font_, TTF_HINTING_NORMAL);
                        fonts_["default"] = default_font_;
                        break;
                    }
                }

                if (!default_font_) {
                    printf("Warning: No system font found, text rendering may be limited\n");
                }
            }
        }

        ~FontManager() {
            if (ttf_initialized_) {
                for (const auto &font: fonts_ | std::views::values) {
                    TTF_CloseFont(font);
                }
                TTF_Quit();
            }
        }

        FontManager(const FontManager &) = delete;

        FontManager &operator=(const FontManager &) = delete;

        bool loadFont(const std::string &name, const std::string &path, const int size) {
            if (!ttf_initialized_) return false;

            TTF_Font *font = TTF_OpenFont(path.c_str(), size);
            if (!font) return false;

            TTF_SetFontHinting(font, TTF_HINTING_NORMAL);

            fonts_[name] = font;
            return true;
        }

        TTF_Font *getFont(const std::string &name = "default") {
            const auto it = fonts_.find(name);
            return it != fonts_.end() ? it->second : default_font_;
        }

        void renderText(const std::string &text, const float x, const float y,
                        const float scale, const Color &color, const std::string &font_name = "default") {
            TTF_Font *font = getFont(font_name);
            if (!font || text.empty()) return;

            int target_size = static_cast<int>(24 * scale);
            target_size = std::max(8, std::min(target_size, 128));

            const std::string sized_font_key = font_name + "_" + std::to_string(target_size);
            TTF_Font *sized_font = getFont(sized_font_key);

            if (!sized_font) {
                const char *font_paths[] = {
                    "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
                    "/usr/share/fonts/TTF/DejaVuSans-Bold.ttf",
                    "/System/Library/Fonts/Arial.ttf",
                    "/Windows/Fonts/arial.ttf",
                    "/usr/share/fonts/truetype/liberation/LiberationSans-Bold.ttf"
                };

                for (const char *path: font_paths) {
                    sized_font = TTF_OpenFont(path, target_size);
                    if (sized_font) {
                        TTF_SetFontHinting(sized_font, TTF_HINTING_NORMAL);
                        fonts_[sized_font_key] = sized_font;
                        break;
                    }
                }

                if (!sized_font) {
                    sized_font = font;
                }
            }

            const SDL_Color sdl_color = {
                static_cast<Uint8>(color.r * 255),
                static_cast<Uint8>(color.g * 255),
                static_cast<Uint8>(color.b * 255),
                static_cast<Uint8>(color.a * 255)
            };

            SDL_Surface *text_surface = TTF_RenderText_Blended(sized_font, text.c_str(), sdl_color);
            if (!text_surface) return;

            GLuint texture;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);

            if (const bool is_integer_scale = (scale == std::floor(scale)) && scale >= 1.0f;
                is_integer_scale && scale <= 4.0f) {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            } else {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            }

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            SDL_Surface *rgba_surface = SDL_ConvertSurfaceFormat(text_surface, SDL_PIXELFORMAT_RGBA32, 0);
            SDL_FreeSurface(text_surface);

            if (rgba_surface) {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, rgba_surface->w, rgba_surface->h,
                             0, GL_RGBA, GL_UNSIGNED_BYTE, rgba_surface->pixels);

                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

                glEnable(GL_TEXTURE_2D);
                glColor4f(1.0f, 1.0f, 1.0f, color.a);

                const auto width = static_cast<float>(rgba_surface->w);
                const auto height = static_cast<float>(rgba_surface->h);

                const int ascent = TTF_FontAscent(sized_font);

                const float aligned_x = std::round(x);
                const float aligned_y = std::round(y);

                const float render_y = aligned_y - ascent;

                glBegin(GL_QUADS);
                glTexCoord2f(0.0f, 1.0f);
                glVertex2f(aligned_x, render_y);
                glTexCoord2f(1.0f, 1.0f);
                glVertex2f(aligned_x + width, render_y);
                glTexCoord2f(1.0f, 0.0f);
                glVertex2f(aligned_x + width, render_y + height);
                glTexCoord2f(0.0f, 0.0f);
                glVertex2f(aligned_x, render_y + height);
                glEnd();

                glDisable(GL_TEXTURE_2D);
                glDisable(GL_BLEND);
                SDL_FreeSurface(rgba_surface);
            }
            glDeleteTextures(1, &texture);
        }

        [[nodiscard]] float getTextWidth(const std::string &text, const float scale = 1.0f,
                                         const std::string &font_name = "default") const {
            if (!ttf_initialized_ || text.empty()) return 0.0f;

            int target_size = static_cast<int>(24 * scale);
            target_size = std::max(8, std::min(target_size, 128));

            const std::string sized_font_key = font_name + "_" + std::to_string(target_size);

            const auto sized_it = fonts_.find(sized_font_key);
            TTF_Font *font = nullptr;
            bool is_sized_font = false;

            if (sized_it != fonts_.end()) {
                font = sized_it->second;
                is_sized_font = true;
            } else {
                const auto default_it = fonts_.find(font_name);
                font = (default_it != fonts_.end()) ? default_it->second : default_font_;
            }

            if (!font) return 0.0f;

            int width;
            TTF_SizeText(font, text.c_str(), &width, nullptr);

            return is_sized_font ? static_cast<float>(width) : width * scale;
        }

        [[nodiscard]] float getTextHeight(const std::string &text, const float scale = 1.0f,
                                          const std::string &font_name = "default") const {
            if (!ttf_initialized_ || text.empty()) return 0.0f;

            int target_size = static_cast<int>(24 * scale);
            target_size = std::max(8, std::min(target_size, 128));

            const std::string sized_font_key = font_name + "_" + std::to_string(target_size);

            const auto sized_it = fonts_.find(sized_font_key);
            TTF_Font *font = nullptr;
            bool is_sized_font = false;

            if (sized_it != fonts_.end()) {
                font = sized_it->second;
                is_sized_font = true;
            } else {
                const auto default_it = fonts_.find(font_name);
                font = (default_it != fonts_.end()) ? default_it->second : default_font_;
            }

            if (!font) return 0.0f;

            int height;
            TTF_SizeText(font, text.c_str(), nullptr, &height);

            return is_sized_font ? static_cast<float>(height) : height * scale;
        }

        [[nodiscard]] bool isInitialized() const {
            return ttf_initialized_ && default_font_ != nullptr;
        }
    };

    class TextRenderer {
        FontManager &font_manager_;

    public:
        explicit TextRenderer(FontManager &fm) : font_manager_(fm) {
        }

        void drawText(const std::string &text, const float x, const float y,
                      const float scale = 1.0f, const Color &color = Color{},
                      const TextAlign align = TextAlign::Left) const {
            if (!font_manager_.isInitialized()) return;

            const float text_width = font_manager_.getTextWidth(text, scale);
            float render_x = x;

            switch (align) {
                case TextAlign::Center:
                    render_x = x - text_width / 2.0f;
                    break;
                case TextAlign::Right:
                    render_x = x - text_width;
                    break;
                case TextAlign::Left:
                default:
                    render_x = x;
                    break;
            }

            font_manager_.renderText(text, render_x, y, scale, color);
        }

        void drawTextCentered(const std::string &text, const float center_x, const float y,
                              const float scale = 1.0f, const Color &color = Color{}) const {
            drawText(text, center_x, y, scale, color, TextAlign::Center);
        }

        [[nodiscard]] float getTextWidth(const std::string &text, const float scale = 1.0f) const {
            return font_manager_.getTextWidth(text, scale);
        }

        [[nodiscard]] float getTextHeight(const std::string &text, const float scale = 1.0f) const {
            return font_manager_.getTextHeight(text, scale);
        }
    };
}
