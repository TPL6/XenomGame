#include "PlayerShip.h"

#include "CollisionLayers.h"
#include "Enemy.h"
#include "EnemyProjectile.h"
#include "Explosion.h"
#include "Missile.h"

#include "engine/InputManager.h"
#include "engine/Level.h"
#include "engine/PhysicsTypes.h"
#include "engine/PhysicsWorld.h"
#include "engine/Renderer.h"
#include "engine/ResourceManager.h"

#include <algorithm>
#include <glm/geometric.hpp>

namespace {

    void destroyShip(Actor* ship) {
        if (ship == nullptr || ship->isPendingDestroy()) {
            return;
        }

        if (ship->level() != nullptr) {
            ship->level()->queueDestroyActor(ship);
        }
        else {
            ship->destroy();
        }
    }

} // namespace

PlayerShip::PlayerShip(glm::vec2 startPos, float playfieldWidth, float playfieldHeight)
    : playfieldWidth_(playfieldWidth)
    , playfieldHeight_(playfieldHeight)
{
    setPosition(startPos);
    setSize({ 64.0f, 64.0f });
    setOrigin({ 0.5f, 0.5f });
    setDrawOrder(20);
}

void PlayerShip::onSpawn(Level& level, ResourceManager& resources) {
    const Texture* tex = resources.loadTexture(
        "ship",
        "graphics/Ship1.bmp",
        true,
        TextureWrapMode::ClampToEdge,
        TextureFilterMode::Nearest
    );

    const TextureHandle handle = (tex && tex->isValid())
        ? static_cast<TextureHandle>(tex->id())
        : kInvalidTextureHandle;

    bankFrameCount_ = 7;
    setSprite(handle, { 64.0f, 64.0f }, bankFrameCount_, 1, 0.0f, false);
    setCurrentFrame(bankFrameCount_ / 2);
    bankTimer_ = 0.0f;

    BodyCreateDesc desc{};
    desc.type = BodyType::Kinematic;
    desc.positionPixels = position();
    desc.rotationRadians = rotation();
    desc.halfSizePixels = { 18.0f, 20.0f };
    desc.density = 0.0f;
    desc.friction = 0.0f;
    desc.restitution = 0.0f;
    desc.fixedRotation = true;
    desc.isSensor = true;
    desc.enableSensorEvents = true;
    desc.filter.categoryBits = CollisionLayers::Player;
    desc.filter.maskBits = CollisionLayers::Enemy | CollisionLayers::EnemyShot;
    desc.userData = this;

    attachBody(level.physics().createBody(desc));
}

void PlayerShip::updateBankingAnimation(float dt, float moveDirX) {
    if (bankFrameCount_ <= 1) {
        return;
    }

    const int centerFrame = bankFrameCount_ / 2;
    int targetFrame = centerFrame;
    if (moveDirX < -0.05f) {
        targetFrame = 0; // First 3 tiles (2 -> 1 -> 0): turning left animation
    }
    else if (moveDirX > 0.05f) {
        targetFrame = bankFrameCount_ - 1; // Last 3 tiles (4 -> 5 -> 6): turning right animation
    }

    const int current = currentFrame();
    if (current == targetFrame) {
        bankTimer_ = 0.0f;
        return;
    }

    bankTimer_ += dt;
    while (bankTimer_ >= bankStepDuration_ && currentFrame() != targetFrame) {
        bankTimer_ -= bankStepDuration_;
        const int step = (targetFrame > currentFrame()) ? 1 : -1;
        setCurrentFrame(currentFrame() + step);
    }
}

void PlayerShip::spawnMissile(Level& level) {
    const glm::vec2 muzzlePos{ position().x, position().y - size().y * 0.5f - 6.0f };
    level.spawnActor<Missile>(muzzlePos, -640.0f);
}

void PlayerShip::update(float dt, const InputManager& input, ResourceManager& resources) {
    if (!alive_ || isPendingDestroy()) {
        return;
    }

    Actor::update(dt, input, resources);

    glm::vec2 moveDir{ 0.0f, 0.0f };
    if (input.isActionDown(GameAction::MoveLeft)) { moveDir.x -= 1.0f; }
    if (input.isActionDown(GameAction::MoveRight)) { moveDir.x += 1.0f; }
    if (input.isActionDown(GameAction::MoveUp)) { moveDir.y -= 1.0f; }
    if (input.isActionDown(GameAction::MoveDown)) { moveDir.y += 1.0f; }

    if (glm::length(moveDir) > 1.0f) {
        moveDir = glm::normalize(moveDir);
    }

    const float halfW = size().x * 0.5f * scale().x;
    const float halfH = size().y * 0.5f * scale().y;
    glm::vec2 nextPos = position() + moveDir * moveSpeed_ * dt;
    nextPos.x = std::clamp(nextPos.x, halfW, std::max(halfW, playfieldWidth_ - halfW));
    nextPos.y = std::clamp(nextPos.y, halfH, std::max(halfH, playfieldHeight_ - halfH));
    setPosition(nextPos);

    updateBankingAnimation(dt, moveDir.x);

    fireCooldown_ = std::max(0.0f, fireCooldown_ - dt);
    if (input.isActionDown(GameAction::Fire) && fireCooldown_ <= 0.0f) {
        fireCooldown_ = fireInterval_;
        if (level() != nullptr) {
            spawnMissile(*level());
        }
    }
}

void PlayerShip::onCollision(const CollisionEvent& event) {
    if (!alive_ || isPendingDestroy()) {
        return;
    }

    if (event.type != CollisionEventType::SensorBegin &&
        event.type != CollisionEventType::BeginContact) {
        return;
    }

    if (dynamic_cast<EnemyProjectile*>(event.actorB) != nullptr ||
        dynamic_cast<Enemy*>(event.actorB) != nullptr)
    {
        alive_ = false;
        if (level() != nullptr) {
            level()->spawnActor<Explosion>(position(), glm::vec2{ 64.0f, 64.0f }, 0.05f);
        }
        destroyShip(this);
    }
}
