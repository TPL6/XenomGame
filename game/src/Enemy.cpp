#include "Enemy.h"

#include "CollisionLayers.h"
#include "Explosion.h"
#include "Missile.h"

#include "engine/Level.h"
#include "engine/PhysicsTypes.h"
#include "engine/PhysicsWorld.h"
#include "engine/ResourceManager.h"

#include <algorithm>

Enemy::Enemy(
    glm::vec2 startPos,
    glm::vec2 spriteSize,
    int drawOrder,
    int maxHp,
    float playfieldHeight)
    : playfieldHeight_(playfieldHeight)
    , hp_(std::max(1, maxHp))
{
    setPosition(startPos);
    setSize(spriteSize);
    setOrigin({ 0.5f, 0.5f });
    setDrawOrder(drawOrder);
}

void Enemy::update(float dt, const InputManager& input, ResourceManager& resources) {
    if (isPendingDestroy()) {
        return;
    }

    Actor::update(dt, input, resources);
    updateBehavior(dt);
}

void Enemy::onCollision(const CollisionEvent& event) {
    if (isPendingDestroy()) {
        return;
    }

    if (event.type != CollisionEventType::SensorBegin &&
        event.type != CollisionEventType::BeginContact) {
        return;
    }

    if (dynamic_cast<Missile*>(event.actorB) != nullptr) {
        takeDamage(1);
    }
}

void Enemy::takeDamage(int amount) {
    if (isPendingDestroy() || hp_ <= 0 || amount <= 0) {
        return;
    }

    hp_ -= amount;
    if (hp_ <= 0) {
        onDestroyed();
    }
}

void Enemy::onDestroyed() {
    if (level() != nullptr) {
        level()->spawnActor<Explosion>(position(), explosionSize_, explosionFrameDuration_);
    }
    destroySelf();
}

TextureHandle Enemy::loadEnemyTexture(
    ResourceManager& resources,
    const std::string& key,
    std::string_view filePath) const
{
    const Texture* tex = resources.loadTexture(
        key,
        filePath,
        true,
        TextureWrapMode::ClampToEdge,
        TextureFilterMode::Nearest
    );

    return (tex && tex->isValid())
        ? static_cast<TextureHandle>(tex->id())
        : kInvalidTextureHandle;
}

void Enemy::attachHitbox(Level& level, glm::vec2 halfSizePixels) {
    BodyCreateDesc desc{};
    desc.type = BodyType::Kinematic;
    desc.positionPixels = position();
    desc.rotationRadians = rotation();
    desc.halfSizePixels = halfSizePixels;
    desc.density = 0.0f;
    desc.friction = 0.0f;
    desc.restitution = 0.0f;
    desc.fixedRotation = true;
    desc.isSensor = true;
    desc.enableSensorEvents = true;
    desc.filter.categoryBits = CollisionLayers::Enemy;
    desc.filter.maskBits = CollisionLayers::Player | CollisionLayers::PlayerShot;
    desc.userData = this;

    attachBody(level.physics().createBody(desc));
}

void Enemy::destroySelf() {
    if (isPendingDestroy()) {
        return;
    }

    if (level() != nullptr) {
        level()->queueDestroyActor(this);
    }
    else {
        destroy();
    }
}
