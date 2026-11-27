#pragma once

#include "engine/PhysicsTypes.h"

#include <cstdint>
#include <glm/vec2.hpp>

// RAII wrapper for a physics body created by PhysicsWorld.
// Automatically destroys the underlying Box2D body when destroyed.
class PhysicsBody {
public:
    PhysicsBody() = default;
    ~PhysicsBody();

    PhysicsBody(const PhysicsBody&) = delete;
    PhysicsBody& operator=(const PhysicsBody&) = delete;

    PhysicsBody(PhysicsBody&& other) noexcept;
    PhysicsBody& operator=(PhysicsBody&& other) noexcept;

    bool isValid() const noexcept;
    void reset() noexcept;

    BodyType type() const noexcept;
    bool isFixedRotation() const noexcept;

    glm::vec2 positionPixels(float pixelsPerMeter) const noexcept;
    float rotationRadians() const noexcept;

    void setTransformPixels(glm::vec2 posPixels, float rotationRadians, float pixelsPerMeter);
    void setTargetTransformPixels(glm::vec2 posPixels, float rotationRadians, float dt, float pixelsPerMeter);

    void setLinearVelocityPixels(glm::vec2 velocityPixelsPerSec, float pixelsPerMeter);
    glm::vec2 linearVelocityPixels(float pixelsPerMeter) const noexcept;

    void setAngularVelocity(float radiansPerSec);
    float angularVelocity() const noexcept;

private:
    friend class PhysicsWorld;

    explicit PhysicsBody(std::uint64_t rawId) noexcept : rawId_(rawId) {}

    std::uint64_t rawId_ = 0;
};
