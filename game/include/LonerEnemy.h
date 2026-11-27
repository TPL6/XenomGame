#pragma once

#include "Enemy.h"

#include <glm/vec2.hpp>

class Level;
class ResourceManager;

class LonerEnemy final : public Enemy {
public:
    LonerEnemy(glm::vec2 startPos, float dirX, float playfieldWidth, float playfieldHeight);

    void onSpawn(Level& level, ResourceManager& resources) override;

protected:
    void updateBehavior(float dt) override;

private:
    void fireAimedShot(Level& level);

    float dirX_ = 1.0f;
    float moveSpeedX_ = 120.0f;
    float playfieldWidth_ = 640.0f;
    float shotTimer_ = 0.0f;
    float shotInterval_ = 2.0f;
};
