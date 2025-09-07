#pragma once
#include "math.hpp"

namespace core {
    class Entity {
    public:
        Vector2 pos{};
        Vector2 size{};
        bool active{true};

        constexpr Entity() = default;

        constexpr Entity(const Vector2 pos, const Vector2 size) : pos(pos), size(size) {
        }

        virtual void update(float dt) = 0;

        virtual void render() = 0;

        virtual ~Entity() = default;

        [[nodiscard]] constexpr Rectangle getBounds() const noexcept {
            return {pos, size};
        }

        [[nodiscard]] constexpr bool collidesWith(const Entity &other) const noexcept {
            return getBounds().intersects(other.getBounds());
        }
    };
}
