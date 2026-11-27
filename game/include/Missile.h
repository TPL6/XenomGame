#pragma once

#include "engine/Actor.h"

#include <glm/vec2.hpp>

class Level;
class InputManager;
class ResourceManager;

class Missile final : public Actor {
public:
    Missile(glm::vec2 startPos, float speedY = -620.0f);

    void onSpawn(Level& level, ResourceManager& resources) override;
    void update(float dt, const InputManager& input, ResourceManager& resources) override;
    void onCollision(const CollisionEvent& event) override;

private:
    float speedY_ = -620.0f;
    float lifeRemaining_ = 2.5f;
};
