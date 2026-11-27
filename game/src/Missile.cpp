#include "Missile.h"

#include "CollisionLayers.h"
#include "Enemy.h"
#include "Explosion.h"

#include "engine/InputManager.h"
#include "engine/Level.h"
#include "engine/PhysicsTypes.h"
#include "engine/PhysicsWorld.h"
#include "engine/Renderer.h"
#include "engine/ResourceManager.h"
#include "engine/SpriteSheet.h"

#include <utility>

namespace {

    void destroyMissile(Actor* actor) {
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

Missile::Missile(glm::vec2 startPos, float speedY)
    : speedY_(speedY)
{
    setPosition(startPos);
    setSize({ 24.0f, 24.0f });
    setOrigin({ 0.5f, 0.5f });
    setDrawOrder(15);
}

void Missile::onSpawn(Level& level, ResourceManager& resources) {
    const Texture* tex = resources.loadTexture(
        "missile",
        "graphics/missile.bmp",
        true,
        TextureWrapMode::ClampToEdge,
        TextureFilterMode::Nearest
    );

    const TextureHandle handle = (tex && tex->isValid())
        ? static_cast<TextureHandle>(tex->id())
        : kInvalidTextureHandle;

    // First row (light missiles) of graphics/missile.bmp (2 columns x 3 rows -> first 2 frames)
    setSprite(handle, { 24.0f, 24.0f }, 2, 3, 0.0f, false);
    setUVFrames(makeSpriteSheetUVs(2, 3, 2), 0.1f, true);

    BodyCreateDesc desc{};
    desc.type = BodyType::Dynamic;
    desc.positionPixels = position();
    desc.rotationRadians = rotation();
    desc.halfSizePixels = { 7.0f, 10.0f };
    desc.density = 1.0f;
    desc.friction = 0.0f;
    desc.restitution = 0.0f;
    desc.fixedRotation = true;
    desc.isSensor = false;
    desc.isBullet = true; // Continuous collision sweeping against moving enemy hitboxes
    desc.enableSensorEvents = true;
    desc.filter.categoryBits = CollisionLayers::PlayerShot;
    desc.filter.maskBits = CollisionLayers::Enemy;
    desc.userData = this;

    PhysicsBody physBody = level.physics().createBody(desc);
    physBody.setLinearVelocityPixels({ 0.0f, speedY_ }, level.physics().pixelsPerMeter());
    attachBody(std::move(physBody));
}

void Missile::update(float dt, const InputManager& input, ResourceManager& resources) {
    if (isPendingDestroy()) {
        return;
    }

    Actor::update(dt, input, resources);

    lifeRemaining_ -= dt;
    if (lifeRemaining_ <= 0.0f || position().y < -40.0f) {
        destroyMissile(this);
    }
}

void Missile::onCollision(const CollisionEvent& event) {
    if (isPendingDestroy()) {
        return;
    }

    if (event.type != CollisionEventType::SensorBegin &&
        event.type != CollisionEventType::BeginContact) {
        return;
    }

    if (dynamic_cast<Enemy*>(event.actorB) != nullptr) {
        if (level() != nullptr) {
            level()->spawnActor<Explosion>(position(), glm::vec2{ 48.0f, 48.0f }, 0.045f);
        }
        destroyMissile(this);
    }
}
