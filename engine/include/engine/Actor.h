#pragma once

#include "engine/Animator.h"
#include "engine/PhysicsBody.h"
#include "engine/PhysicsTypes.h"
#include "engine/Renderer.h"

#include <cstdint>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <utility>
#include <vector>

class InputManager;
class Level;
class ResourceManager;

class Actor {
public:
    using ActorId = std::uint64_t;

    Actor() = default;
    virtual ~Actor() = default;

    Actor(const Actor&) = delete;
    Actor& operator=(const Actor&) = delete;

    ActorId id() const noexcept { return id_; }
    Level* level() const noexcept { return level_; }

    bool isPendingDestroy() const noexcept { return pendingDestroy_; }
    void destroy() noexcept { pendingDestroy_ = true; }

    // Transform in world/screen pixels and radians
    glm::vec2 position() const noexcept { return position_; }
    void setPosition(glm::vec2 pos) noexcept { position_ = pos; }

    float rotation() const noexcept { return rotationRadians_; }
    void setRotation(float radians) noexcept { rotationRadians_ = radians; }

    glm::vec2 scale() const noexcept { return scale_; }
    void setScale(glm::vec2 sc) noexcept { scale_ = sc; }

    glm::vec2 size() const noexcept { return size_; }
    void setSize(glm::vec2 sz) noexcept { size_ = sz; }

    glm::vec2 origin() const noexcept { return origin_; }
    void setOrigin(glm::vec2 orig) noexcept { origin_ = orig; }

    glm::vec4 tint() const noexcept { return tint_; }
    void setTint(glm::vec4 color) noexcept { tint_ = color; }

    // Sprite & grid animation support (delegated to Animator)
    void setSprite(
        TextureHandle texture,
        glm::vec2 quadSize,
        int columns = 1,
        int rows = 1,
        float frameDuration = 0.1f,
        bool loop = true,
        int maxFrames = 0
    );
    void setUVFrames(std::vector<UVRect> frames, float frameDuration = 0.1f, bool loop = true);
    void updateAnimation(float dt);
    void setCurrentFrame(int index);
    int currentFrame() const noexcept { return animator_.currentFrame(); }
    bool isAnimationFinished() const noexcept { return animator_.isFinished(); }

    // Optional RAII physics body binding
    void attachBody(PhysicsBody body) noexcept { body_ = std::move(body); }
    PhysicsBody& body() noexcept { return body_; }
    const PhysicsBody& body() const noexcept { return body_; }
    bool hasBody() const noexcept { return body_.isValid(); }

    void syncFromPhysics(float pixelsPerMeter);
    void syncToPhysics(float pixelsPerMeter);
    void syncTargetToPhysics(float dt, float pixelsPerMeter);

    // Lifecycle & gameplay hooks
    virtual void onSpawn(Level& /*level*/, ResourceManager& /*resources*/) {}
    virtual void onDespawn() {}
    virtual void update(float dt, const InputManager& input, ResourceManager& resources);
    virtual void render(Renderer& renderer) const;
    virtual void onCollision(const CollisionEvent& /*event*/) {}

protected:
    void setDrawOrder(int order) noexcept { drawOrder_ = order; }

private:
    friend class Level;

    ActorId id_ = 0;
    Level* level_ = nullptr;
    int drawOrder_ = 0;
    bool pendingDestroy_ = false;

    glm::vec2 position_{ 0.0f, 0.0f };
    float rotationRadians_ = 0.0f;
    glm::vec2 scale_{ 1.0f, 1.0f };
    glm::vec2 size_{ 32.0f, 32.0f };
    glm::vec2 origin_{ 0.5f, 0.5f };
    glm::vec4 tint_{ 1.0f, 1.0f, 1.0f, 1.0f };

    TextureHandle texture_ = kInvalidTextureHandle;
    Animator animator_;

    PhysicsBody body_;
};
