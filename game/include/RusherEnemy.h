#pragma once

#include "Enemy.h"

#include <glm/vec2.hpp>

class Level;
class ResourceManager;

class RusherEnemy final : public Enemy {
public:
    RusherEnemy(glm::vec2 startPos, float rushSpeedY = 250.0f, float playfieldHeight = 480.0f);

    void onSpawn(Level& level, ResourceManager& resources) override;

protected:
    void updateBehavior(float dt) override;

private:
    float rushSpeedY_ = 250.0f;
    float swayAmplitude_ = 40.0f;
    float swayFrequency_ = 3.5f;
    float elapsed_ = 0.0f;
};
