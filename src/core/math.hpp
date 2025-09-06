#pragma once
#include <cmath>

namespace core {

struct Vector2 {
    float x{}, y{};
    
    constexpr Vector2() = default;
    constexpr Vector2(float x, float y) : x(x), y(y) {}
    
    constexpr Vector2 operator+(const Vector2& other) const noexcept {
        return {x + other.x, y + other.y};
    }
    
    constexpr Vector2& operator+=(const Vector2& other) noexcept {
        x += other.x;
        y += other.y;
        return *this;
    }
    
    constexpr Vector2 operator*(float scalar) const noexcept {
        return {x * scalar, y * scalar};
    }
    
    constexpr Vector2& operator*=(float scalar) noexcept {
        x *= scalar;
        y *= scalar;
        return *this;
    }
    
    [[nodiscard]] constexpr float dot(const Vector2& other) const noexcept {
        return x * other.x + y * other.y;
    }
    
    [[nodiscard]] float length() const noexcept {
        return std::sqrt(x * x + y * y);
    }
    
    [[nodiscard]] Vector2 normalized() const noexcept {
        const float len = length();
        return len > 0 ? Vector2{x / len, y / len} : Vector2{};
    }
};

struct Rectangle {
    Vector2 pos{};
    Vector2 size{};
    
    constexpr Rectangle() = default;
    constexpr Rectangle(Vector2 pos, Vector2 size) : pos(pos), size(size) {}
    
    [[nodiscard]] constexpr bool contains(const Vector2& point) const noexcept {
        return point.x >= pos.x - size.x/2 && point.x <= pos.x + size.x/2 &&
               point.y >= pos.y - size.y/2 && point.y <= pos.y + size.y/2;
    }
    
    [[nodiscard]] constexpr bool intersects(const Rectangle& other) const noexcept {
        return !(pos.x + size.x/2 < other.pos.x - other.size.x/2 ||
                 pos.x - size.x/2 > other.pos.x + other.size.x/2 ||
                 pos.y + size.y/2 < other.pos.y - other.size.y/2 ||
                 pos.y - size.y/2 > other.pos.y + other.size.y/2);
    }
};

} // namespace core
