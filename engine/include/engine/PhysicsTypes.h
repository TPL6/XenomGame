#pragma once

#include <cstdint>
#include <glm/vec2.hpp>

class Actor;

enum class BodyType : std::uint8_t {
    Static,
    Kinematic,
    Dynamic
};

enum class CollisionEventType : std::uint8_t {
    BeginContact,
    EndContact,
    Hit,
    SensorBegin,
    SensorEnd
};

struct CollisionFilter {
    std::uint64_t categoryBits = 0x0000000000000001ULL;
    std::uint64_t maskBits = 0xFFFFFFFFFFFFFFFFULL;
    int groupIndex = 0;
};

struct CollisionEvent {
    CollisionEventType type = CollisionEventType::BeginContact;
    Actor* actorA = nullptr;
    Actor* actorB = nullptr;
    glm::vec2 contactPoint{ 0.0f, 0.0f };
    glm::vec2 normal{ 0.0f, 0.0f };
    float approachSpeed = 0.0f;
};

struct BodyCreateDesc {
    BodyType type = BodyType::Dynamic;
    glm::vec2 positionPixels{ 0.0f, 0.0f };
    float rotationRadians = 0.0f;
    glm::vec2 halfSizePixels{ 16.0f, 16.0f }; // Used for box shape if radiusPixels <= 0
    float radiusPixels = 0.0f;               // If > 0, creates a circle shape instead
    float density = 1.0f;
    float friction = 0.3f;
    float restitution = 0.0f;
    bool fixedRotation = false;
    bool isSensor = false;
    bool isBullet = false;
    bool enableSensorEvents = true;
    CollisionFilter filter{};
    void* userData = nullptr;
};
