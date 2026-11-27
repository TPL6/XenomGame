#pragma once

#include "engine/Actor.h"

#include <glm/vec2.hpp>

class Level;
class InputManager;
class ResourceManager;

class PlayerShip final : public Actor {
public:
    PlayerShip(glm::vec2 startPos, float playfieldWidth, float playfieldHeight);

    void onSpawn(Level& level, ResourceManager& resources) override;
    void update(float dt, const InputManager& input, ResourceManager& resources) override;
    void onCollision(const CollisionEvent& event) override;

    bool isAlive() const noexcept { return alive_; }

private:
    void spawnMissile(Level& level);
    void updateBankingAnimation(float dt, float moveDirX);

    float playfieldWidth_ = 640.0f;
    float playfieldHeight_ = 480.0f;
    float moveSpeed_ = 280.0f;
    float fireCooldown_ = 0.0f;
    float fireInterval_ = 0.16f;
    int bankFrameCount_ = 7;
    float bankTimer_ = 0.0f;
    float bankStepDuration_ = 0.06f;
    bool alive_ = true;
};
