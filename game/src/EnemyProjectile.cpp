#include "EnemyProjectile.h"

#include "CollisionLayers.h"
#include "Explosion.h"
#include "PlayerShip.h"

#include "engine/InputManager.h"
#include "engine/Level.h"
#include "engine/PhysicsTypes.h"
#include "engine/PhysicsWorld.h"
#include "engine/Renderer.h"
#include "engine/ResourceManager.h"

#include <glm/geometric.hpp>

namespace {

    constexpr float kOffscreenMargin = 64.0f;

    void destroyOrQueue(Actor* actor) {
        if (actor == nullptr || actor->isPendingDestroy()) {
            return;
        }

        if (actor->level() != nullptr) {
            actor->level()->queueDestroyActor(actor);
        }
        else {
            actor->destroy();
        }
    }

} // namespace

EnemyProjectile::EnemyProjectile(glm::vec2 startPos, glm::vec2 velocity, float speed)
    : velocity_(velocity)
    , speed_(speed)
{
    setPosition(startPos);
    setSize({ 24.0f, 24.0f });
    setOrigin({ 0.5f, 0.5f });
    setDrawOrder(16);

    if (glm::length(velocity_) > 0.001f) {
        velocity_ = glm::normalize(velocity_) * speed_;
    }
    else {
        velocity_ = { 0.0f, speed_ };
    }
}

void EnemyProjectile::onSpawn(Level& level, ResourceManager& resources) {
    // Uses graphics/EnWeap6.bmp (128x16 = 8x1 animated frames) with visible rotation
    const Texture* tex = resources.loadTexture(
        "enemy_proj",
        "graphics/EnWeap6.bmp",
        true,
        TextureWrapMode::ClampToEdge,
        TextureFilterMode::Nearest
    );

    const TextureHandle handle = (tex && tex->isValid())
        ? static_cast<TextureHandle>(tex->id())
        : kInvalidTextureHandle;

    setSprite(handle, { 24.0f, 24.0f }, 8, 1, 0.07f, true);

    BodyCreateDesc desc{};
    desc.type = BodyType::Dynamic;
    desc.positionPixels = position();
    desc.rotationRadians = rotation();
    desc.radiusPixels = 8.0f;
    desc.density = 1.0f;
    desc.friction = 0.0f;
    desc.restitution = 0.0f;
    desc.fixedRotation = false; // Allow angular velocity for visible rotation
    desc.isSensor = false;
    desc.isBullet = true; // Continuous collision sweeping against player hitbox
    desc.enableSensorEvents = true;
    desc.filter.categoryBits = CollisionLayers::EnemyShot;
    desc.filter.maskBits = CollisionLayers::Player;
    desc.userData = this;

    PhysicsBody physBody = level.physics().createBody(desc);
    physBody.setLinearVelocityPixels(velocity_, level.physics().pixelsPerMeter());
    physBody.setAngularVelocity(spinSpeed_);
    attachBody(std::move(physBody));
}

void EnemyProjectile::update(float dt, const InputManager& input, ResourceManager& resources) {
    if (isPendingDestroy()) {
        return;
    }

    Actor::update(dt, input, resources);

    lifeRemaining_ -= dt;
    const glm::vec2 pos = position();

    if (lifeRemaining_ <= 0.0f ||
        pos.x < -kOffscreenMargin || pos.x > 2000.0f ||
        pos.y < -kOffscreenMargin || pos.y > 2000.0f)
    {
        destroyOrQueue(this);
    }
}

void EnemyProjectile::onCollision(const CollisionEvent& event) {
    if (isPendingDestroy()) {
        return;
    }

    if (event.type != CollisionEventType::SensorBegin &&
        event.type != CollisionEventType::BeginContact) {
        return;
    }

    if (dynamic_cast<PlayerShip*>(event.actorB) != nullptr) {
        if (level() != nullptr) {
            level()->spawnActor<Explosion>(position(), glm::vec2{ 48.0f, 48.0f }, 0.045f);
        }
        destroyOrQueue(this);
    }
}
