#pragma once

#include "engine/Actor.h"

#include <glm/vec2.hpp>

class InputManager;
class Level;
class ResourceManager;

class EnemyProjectile final : public Actor {
public:
    EnemyProjectile(glm::vec2 startPos, glm::vec2 velocity, float speed = 250.0f);

    void onSpawn(Level& level, ResourceManager& resources) override;
    void update(float dt, const InputManager& input, ResourceManager& resources) override;
    void onCollision(const CollisionEvent& event) override;

private:
    glm::vec2 velocity_{ 0.0f, 250.0f };
    float speed_ = 250.0f;
    float spinSpeed_ = 7.0f; // radians/sec for visible rotation
    float lifeRemaining_ = 5.0f;
};
