#include "LonerEnemy.h"

#include "EnemyProjectile.h"
#include "PlayerShip.h"

#include "engine/Level.h"
#include "engine/ResourceManager.h"

#include <glm/geometric.hpp>

LonerEnemy::LonerEnemy(glm::vec2 startPos, float dirX, float playfieldWidth, float playfieldHeight)
    : Enemy(startPos, { 64.0f, 64.0f }, 18, 2, playfieldHeight)
    , dirX_(dirX >= 0.0f ? 1.0f : -1.0f)
    , playfieldWidth_(playfieldWidth)
{
}

void LonerEnemy::onSpawn(Level& level, ResourceManager& resources) {
    const TextureHandle handle = loadEnemyTexture(resources, "loner", "graphics/LonerA.bmp");
    setSprite(handle, { 64.0f, 64.0f }, 4, 4, 0.08f, true);
    attachHitbox(level, { 22.0f, 22.0f });
}

void LonerEnemy::fireAimedShot(Level& level) {
    glm::vec2 aimDir{ 0.0f, 1.0f };

    for (const auto& actorPtr : level.actors()) {
        if (auto* ship = dynamic_cast<PlayerShip*>(actorPtr.get());
            ship && ship->isAlive() && !ship->isPendingDestroy())
        {
            const glm::vec2 delta = ship->position() - position();
            const float len = glm::length(delta);
            if (len > 0.001f) {
                aimDir = delta / len;
            }
            break;
        }
    }

    const glm::vec2 spawnPos = position() + aimDir * 24.0f;
    level.spawnActor<EnemyProjectile>(spawnPos, aimDir * 240.0f, 240.0f);
}

void LonerEnemy::updateBehavior(float dt) {
    glm::vec2 pos = position();
    pos.x += dirX_ * moveSpeedX_ * dt;
    setPosition(pos);

    const float halfW = size().x * 0.5f;

    // Disappear when reaching/exiting the opposite horizontal wall (no bouncing)
    if ((dirX_ > 0.0f && pos.x >= playfieldWidth_ + halfW) ||
        (dirX_ < 0.0f && pos.x <= -halfW) ||
        pos.y > playfieldHeight() + kOffscreenMargin)
    {
        destroySelf();
        return;
    }

    // Fire an aimed projectile towards the player every 2 seconds while on-screen
    if (pos.x > 16.0f && pos.x < playfieldWidth_ - 16.0f &&
        pos.y > 16.0f && pos.y < playfieldHeight() - 16.0f)
    {
        shotTimer_ += dt;
        if (shotTimer_ >= shotInterval_) {
            shotTimer_ -= shotInterval_;
            if (level() != nullptr) {
                fireAimedShot(*level());
            }
        }
    }
}
