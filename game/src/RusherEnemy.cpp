#include "RusherEnemy.h"

#include "engine/Level.h"
#include "engine/ResourceManager.h"

#include <cmath>

RusherEnemy::RusherEnemy(glm::vec2 startPos, float rushSpeedY, float playfieldHeight)
    : Enemy(startPos, { 64.0f, 32.0f }, 19, 1, playfieldHeight)
    , rushSpeedY_(rushSpeedY)
{
}

void RusherEnemy::onSpawn(Level& level, ResourceManager& resources) {
    const TextureHandle handle = loadEnemyTexture(resources, "rusher", "graphics/rusher.bmp");
    setSprite(handle, { 64.0f, 32.0f }, 4, 6, 0.06f, true);
    attachHitbox(level, { 22.0f, 13.0f });
}

void RusherEnemy::updateBehavior(float dt) {
    elapsed_ += dt;
    glm::vec2 pos = position();
    pos.y += rushSpeedY_ * dt;
    pos.x += std::cos(elapsed_ * swayFrequency_) * swayAmplitude_ * dt;
    setPosition(pos);

    if (pos.y > playfieldHeight() + kOffscreenMargin) {
        destroySelf();
    }
}
