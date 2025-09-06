#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <GL/gl.h>
#include <string>
#include <string_view>
#include <memory>
#include <unordered_map>
#include <cmath>
#include <algorithm>

namespace core {

struct Color {
    float r, g, b, a;
    constexpr Color(float r = 1.0f, float g = 1.0f, float b = 1.0f, float a = 1.0f)
        : r(r), g(g), b(b), a(a) {}
};

enum class TextAlign {
    Left,
    Center,
    Right
};

class FontManager {
private:
    std::unordered_map<std::string, TTF_Font*> fonts_;
    bool ttf_initialized_{false};
    TTF_Font* default_font_{nullptr};
    
public:
    FontManager() {
        if (TTF_Init() == 0) {
            ttf_initialized_ = true;
            
            // Try to load a system font as default with improved hinting
            const char* font_paths[] = {
                "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
                "/usr/share/fonts/TTF/DejaVuSans-Bold.ttf",
                "/System/Library/Fonts/Arial.ttf",
                "/Windows/Fonts/arial.ttf",
                "/usr/share/fonts/truetype/liberation/LiberationSans-Bold.ttf"
            };
            
            for (const char* path : font_paths) {
                default_font_ = TTF_OpenFont(path, 24);
                if (default_font_) {
                    // Set font hinting for better quality
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
            for (auto& [name, font] : fonts_) {
                TTF_CloseFont(font);
            }
            TTF_Quit();
        }
    }
    
    // Delete copy constructor and assignment
    FontManager(const FontManager&) = delete;
    FontManager& operator=(const FontManager&) = delete;
    
    bool loadFont(const std::string& name, const std::string& path, int size) {
        if (!ttf_initialized_) return false;
        
        TTF_Font* font = TTF_OpenFont(path.c_str(), size);
        if (!font) return false;
        
        // Set font hinting for better quality
        TTF_SetFontHinting(font, TTF_HINTING_NORMAL);
        
        fonts_[name] = font;
        return true;
    }
    
    TTF_Font* getFont(const std::string& name = "default") {
        auto it = fonts_.find(name);
        return it != fonts_.end() ? it->second : default_font_;
    }
    
    void renderText(const std::string& text, float x, float y, 
                   float scale, const Color& color, const std::string& font_name = "default") {
        TTF_Font* font = getFont(font_name);
        if (!font || text.empty()) return;
        
        // Calculate the actual size we want to render at
        int target_size = static_cast<int>(24 * scale);
        target_size = std::max(8, std::min(target_size, 128)); // Clamp between 8 and 128
        
        // Create a font at the target size for better quality
        std::string sized_font_key = font_name + "_" + std::to_string(target_size);
        TTF_Font* sized_font = getFont(sized_font_key);
        
        if (!sized_font) {
            // Find the font path and create a new size
            const char* font_paths[] = {
                "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
                "/usr/share/fonts/TTF/DejaVuSans-Bold.ttf",
                "/System/Library/Fonts/Arial.ttf",
                "/Windows/Fonts/arial.ttf",
                "/usr/share/fonts/truetype/liberation/LiberationSans-Bold.ttf"
            };
            
            for (const char* path : font_paths) {
                sized_font = TTF_OpenFont(path, target_size);
                if (sized_font) {
                    // Set font hinting for better quality
                    TTF_SetFontHinting(sized_font, TTF_HINTING_NORMAL);
                    fonts_[sized_font_key] = sized_font;
                    break;
                }
            }
            
            if (!sized_font) {
                sized_font = font; // Fallback to original font
            }
        }
        
        // Convert color to SDL format
        SDL_Color sdl_color = {
            static_cast<Uint8>(color.r * 255),
            static_cast<Uint8>(color.g * 255), 
            static_cast<Uint8>(color.b * 255),
            static_cast<Uint8>(color.a * 255)
        };
        
        // Use high-quality text rendering
        SDL_Surface* text_surface = TTF_RenderText_Blended(sized_font, text.c_str(), sdl_color);
        if (!text_surface) return;
        
        // Create OpenGL texture with better filtering
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        
        // Use nearest neighbor for pixel-perfect scaling when scale is integer
        bool is_integer_scale = (scale == std::floor(scale)) && scale >= 1.0f;
        if (is_integer_scale && scale <= 4.0f) {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        } else {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }
        
        // Ensure texture wrapping doesn't cause artifacts
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
        // Convert surface to RGBA if needed
        SDL_Surface* rgba_surface = SDL_ConvertSurfaceFormat(text_surface, SDL_PIXELFORMAT_RGBA32, 0);
        SDL_FreeSurface(text_surface);
        
        if (rgba_surface) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, rgba_surface->w, rgba_surface->h, 
                        0, GL_RGBA, GL_UNSIGNED_BYTE, rgba_surface->pixels);
            
            // Enable blending for proper alpha
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            
            // Enable texturing
            glEnable(GL_TEXTURE_2D);
            glColor4f(1.0f, 1.0f, 1.0f, color.a);
            
            // Use the surface dimensions directly since we already sized the font
            float width = static_cast<float>(rgba_surface->w);
            float height = static_cast<float>(rgba_surface->h);
            
            // Get font metrics for proper baseline positioning
            int ascent = TTF_FontAscent(sized_font);
            
            // Align to pixel boundaries for crisp rendering
            float aligned_x = std::round(x);
            float aligned_y = std::round(y);
            
            // Adjust Y position to treat the passed Y as the top of the text
            // rather than the baseline. This makes positioning more intuitive.
            float render_y = aligned_y - ascent;
            
            // Render textured quad with proper positioning
            // Now Y coordinate represents the top of the text area
            glBegin(GL_QUADS);
            glTexCoord2f(0.0f, 1.0f); glVertex2f(aligned_x, render_y);              // Bottom-left
            glTexCoord2f(1.0f, 1.0f); glVertex2f(aligned_x + width, render_y);     // Bottom-right  
            glTexCoord2f(1.0f, 0.0f); glVertex2f(aligned_x + width, render_y + height); // Top-right
            glTexCoord2f(0.0f, 0.0f); glVertex2f(aligned_x, render_y + height);    // Top-left
            glEnd();
            
            glDisable(GL_TEXTURE_2D);
            glDisable(GL_BLEND);
            SDL_FreeSurface(rgba_surface);
        }
        
        glDeleteTextures(1, &texture);
    }
    
    [[nodiscard]] float getTextWidth(const std::string& text, float scale = 1.0f, 
                                    const std::string& font_name = "default") const {
        if (!ttf_initialized_ || text.empty()) return 0.0f;
        
        // Calculate the target size for accurate measurement
        int target_size = static_cast<int>(24 * scale);
        target_size = std::max(8, std::min(target_size, 128));
        
        std::string sized_font_key = font_name + "_" + std::to_string(target_size);
        
        // Try to find the sized font first
        auto sized_it = fonts_.find(sized_font_key);
        TTF_Font* font = nullptr;
        bool is_sized_font = false;
        
        if (sized_it != fonts_.end()) {
            font = sized_it->second;
            is_sized_font = true;
        } else {
            auto default_it = fonts_.find(font_name);
            font = (default_it != fonts_.end()) ? default_it->second : default_font_;
        }
        
        if (!font) return 0.0f;
        
        int width;
        TTF_SizeText(font, text.c_str(), &width, nullptr);
        
        // If we're using a sized font, return the actual width
        // Otherwise, apply scale to the default font
        return is_sized_font ? static_cast<float>(width) : width * scale;
    }
    
    [[nodiscard]] float getTextHeight(const std::string& text, float scale = 1.0f,
                                     const std::string& font_name = "default") const {
        if (!ttf_initialized_ || text.empty()) return 0.0f;
        
        // Calculate the target size for accurate measurement
        int target_size = static_cast<int>(24 * scale);
        target_size = std::max(8, std::min(target_size, 128));
        
        std::string sized_font_key = font_name + "_" + std::to_string(target_size);
        
        // Try to find the sized font first
        auto sized_it = fonts_.find(sized_font_key);
        TTF_Font* font = nullptr;
        bool is_sized_font = false;
        
        if (sized_it != fonts_.end()) {
            font = sized_it->second;
            is_sized_font = true;
        } else {
            auto default_it = fonts_.find(font_name);
            font = (default_it != fonts_.end()) ? default_it->second : default_font_;
        }
        
        if (!font) return 0.0f;
        
        int height;
        TTF_SizeText(font, text.c_str(), nullptr, &height);
        
        // If we're using a sized font, return the actual height
        // Otherwise, apply scale to the default font
        return is_sized_font ? static_cast<float>(height) : height * scale;
    }
    
    [[nodiscard]] bool isInitialized() const {
        return ttf_initialized_ && default_font_ != nullptr;
    }
};

class TextRenderer {
private:
    FontManager& font_manager_;
    
public:
    explicit TextRenderer(FontManager& fm) : font_manager_(fm) {}
    
    void drawText(const std::string& text, float x, float y, 
                  float scale = 1.0f, const Color& color = Color{},
                  TextAlign align = TextAlign::Left) {
        if (!font_manager_.isInitialized()) return;
        
        float text_width = font_manager_.getTextWidth(text, scale);
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
    
    void drawTextCentered(const std::string& text, float center_x, float y,
                         float scale = 1.0f, const Color& color = Color{}) {
        drawText(text, center_x, y, scale, color, TextAlign::Center);
    }
    
    [[nodiscard]] float getTextWidth(const std::string& text, float scale = 1.0f) const {
        return font_manager_.getTextWidth(text, scale);
    }
    
    [[nodiscard]] float getTextHeight(const std::string& text, float scale = 1.0f) const {
        return font_manager_.getTextHeight(text, scale);
    }
};

} // namespace core
