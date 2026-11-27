#pragma once

#include "engine/Actor.h"

#include <glm/vec2.hpp>
#include <string>
#include <string_view>

class InputManager;
class Level;
class ResourceManager;

class Enemy : public Actor {
public:
    Enemy(
        glm::vec2 startPos,
        glm::vec2 spriteSize,
        int drawOrder,
        int maxHp,
        float playfieldHeight = 480.0f
    );
    ~Enemy() override = default;

    void update(float dt, const InputManager& input, ResourceManager& resources) override;
    void onCollision(const CollisionEvent& event) override;

    virtual void takeDamage(int amount);

    int hp() const noexcept { return hp_; }
    bool isAlive() const noexcept { return hp_ > 0 && !isPendingDestroy(); }

protected:
    static constexpr float kOffscreenMargin = 64.0f;

    virtual void updateBehavior(float dt) = 0;
    virtual void onDestroyed();

    TextureHandle loadEnemyTexture(
        ResourceManager& resources,
        const std::string& key,
        std::string_view filePath
    ) const;

    void attachHitbox(Level& level, glm::vec2 halfSizePixels);
    void destroySelf();

    float playfieldHeight() const noexcept { return playfieldHeight_; }
    void setExplosionConfig(glm::vec2 quadSize, float frameDuration) noexcept {
        explosionSize_ = quadSize;
        explosionFrameDuration_ = frameDuration;
    }

private:
    float playfieldHeight_ = 480.0f;
    int hp_ = 1;
    glm::vec2 explosionSize_{ 64.0f, 64.0f };
    float explosionFrameDuration_ = 0.05f;
};
