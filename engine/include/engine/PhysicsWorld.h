#pragma once

#include "engine/PhysicsBody.h"
#include "engine/PhysicsTypes.h"

#include <functional>
#include <glm/vec2.hpp>
#include <memory>
#include <vector>

// RAII physics simulation world encapsulating Box2D (v3.2).
class PhysicsWorld {
public:
    using CollisionCallback = std::function<void(const CollisionEvent&)>;

    explicit PhysicsWorld(
        glm::vec2 gravityMetersPerSec2 = { 0.0f, 0.0f },
        float fixedDeltaTime = 1.0f / 60.0f,
        int subStepCount = 4,
        float pixelsPerMeter = 32.0f
    );
    ~PhysicsWorld();

    PhysicsWorld(const PhysicsWorld&) = delete;
    PhysicsWorld& operator=(const PhysicsWorld&) = delete;

    PhysicsWorld(PhysicsWorld&& other) noexcept;
    PhysicsWorld& operator=(PhysicsWorld&& other) noexcept;

    bool isValid() const noexcept;

    void setGravity(glm::vec2 gravityMetersPerSec2);
    glm::vec2 gravity() const;

    void setPixelsPerMeter(float ppm) noexcept;
    float pixelsPerMeter() const noexcept;

    glm::vec2 toMeters(glm::vec2 pixels) const noexcept;
    glm::vec2 toPixels(glm::vec2 meters) const noexcept;

    // Accumulates frame delta and steps fixed physics ticks; returns number of steps executed.
    int update(float deltaTime, const CollisionCallback& onCollision = nullptr);

    // Executes a single fixed step immediately and dispatches collision events.
    void stepFixed(float dt, const CollisionCallback& onCollision = nullptr);

    // Creates a physics body wrapped in an RAII PhysicsBody handle.
    PhysicsBody createBody(const BodyCreateDesc& desc);

    const std::vector<CollisionEvent>& lastStepEvents() const noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
