#include "engine/PhysicsWorld.h"

#include "engine/Actor.h"

#include <algorithm>
#include <box2d/box2d.h>
#include <cmath>
#include <cstddef>
#include <utility>
#include <vector>

namespace {

    b2BodyType toB2BodyType(BodyType type) noexcept {
        switch (type) {
        case BodyType::Static:
            return b2_staticBody;
        case BodyType::Kinematic:
            return b2_kinematicBody;
        case BodyType::Dynamic:
        default:
            return b2_dynamicBody;
        }
    }

    BodyType fromB2BodyType(b2BodyType type) noexcept {
        switch (type) {
        case b2_staticBody:
            return BodyType::Static;
        case b2_kinematicBody:
            return BodyType::Kinematic;
        case b2_dynamicBody:
        default:
            return BodyType::Dynamic;
        }
    }

    Actor* extractActor(b2ShapeId shapeId) {
        if (!b2Shape_IsValid(shapeId)) {
            return nullptr;
        }

        if (void* shapeUserData = b2Shape_GetUserData(shapeId); shapeUserData != nullptr) {
            return static_cast<Actor*>(shapeUserData);
        }

        const b2BodyId bodyId = b2Shape_GetBody(shapeId);
        if (b2Body_IsValid(bodyId)) {
            if (void* bodyUserData = b2Body_GetUserData(bodyId); bodyUserData != nullptr) {
                return static_cast<Actor*>(bodyUserData);
            }
        }

        return nullptr;
    }

    b2ShapeId createShapeForGeometry(
        b2BodyId bodyId,
        const b2ShapeDef& shapeDef,
        glm::vec2 halfSizePixels,
        float radiusPixels,
        float ppm)
    {
        if (radiusPixels > 0.0f) {
            b2Circle circle{};
            circle.center = b2Vec2{ 0.0f, 0.0f };
            circle.radius = std::max(0.01f, radiusPixels / ppm);
            return b2CreateCircleShape(bodyId, &shapeDef, &circle);
        }

        const float hx = std::max(0.01f, halfSizePixels.x / ppm);
        const float hy = std::max(0.01f, halfSizePixels.y / ppm);
        const b2Polygon box = b2MakeBox(hx, hy);
        return b2CreatePolygonShape(bodyId, &shapeDef, &box);
    }

    // RAII wrapper for b2WorldId (following Box2D Modern C++ pattern).
    class Box2DWorld {
    public:
        explicit Box2DWorld(b2Vec2 gravity) {
            b2WorldDef worldDef = b2DefaultWorldDef();
            worldDef.gravity = gravity;
            worldId_ = b2CreateWorld(&worldDef);
        }

        ~Box2DWorld() {
            release();
        }

        Box2DWorld(const Box2DWorld&) = delete;
        Box2DWorld& operator=(const Box2DWorld&) = delete;

        Box2DWorld(Box2DWorld&& other) noexcept
            : worldId_(other.worldId_) {
            other.worldId_ = b2_nullWorldId;
        }

        Box2DWorld& operator=(Box2DWorld&& other) noexcept {
            if (this != &other) {
                release();
                worldId_ = other.worldId_;
                other.worldId_ = b2_nullWorldId;
            }
            return *this;
        }

        bool isValid() const noexcept { return b2World_IsValid(worldId_); }
        b2WorldId id() const noexcept { return worldId_; }

    private:
        void release() noexcept {
            if (b2World_IsValid(worldId_)) {
                b2DestroyWorld(worldId_);
            }
            worldId_ = b2_nullWorldId;
        }

        b2WorldId worldId_ = b2_nullWorldId;
    };

} // namespace

// ---------------- PhysicsBody (RAII) ----------------

PhysicsBody::~PhysicsBody() {
    reset();
}

PhysicsBody::PhysicsBody(PhysicsBody&& other) noexcept
    : rawId_(other.rawId_) {
    other.rawId_ = 0;
}

PhysicsBody& PhysicsBody::operator=(PhysicsBody&& other) noexcept {
    if (this != &other) {
        reset();
        rawId_ = other.rawId_;
        other.rawId_ = 0;
    }
    return *this;
}

bool PhysicsBody::isValid() const noexcept {
    if (rawId_ == 0) {
        return false;
    }
    return b2Body_IsValid(b2LoadBodyId(rawId_));
}

void PhysicsBody::reset() noexcept {
    if (rawId_ != 0) {
        const b2BodyId id = b2LoadBodyId(rawId_);
        if (b2Body_IsValid(id)) {
            b2DestroyBody(id);
        }
        rawId_ = 0;
    }
}

BodyType PhysicsBody::type() const noexcept {
    if (!isValid()) {
        return BodyType::Static;
    }
    return fromB2BodyType(b2Body_GetType(b2LoadBodyId(rawId_)));
}

bool PhysicsBody::isFixedRotation() const noexcept {
    if (!isValid()) {
        return false;
    }
    return b2Body_GetMotionLocks(b2LoadBodyId(rawId_)).angularZ;
}

glm::vec2 PhysicsBody::positionPixels(float pixelsPerMeter) const noexcept {
    if (!isValid() || pixelsPerMeter <= 0.0f) {
        return { 0.0f, 0.0f };
    }
    const b2Pos pos = b2Body_GetPosition(b2LoadBodyId(rawId_));
    return { pos.x * pixelsPerMeter, pos.y * pixelsPerMeter };
}

float PhysicsBody::rotationRadians() const noexcept {
    if (!isValid()) {
        return 0.0f;
    }
    return b2Rot_GetAngle(b2Body_GetRotation(b2LoadBodyId(rawId_)));
}

void PhysicsBody::setTransformPixels(glm::vec2 posPixels, float rotationRadians, float pixelsPerMeter) {
    if (!isValid() || pixelsPerMeter <= 0.0f) {
        return;
    }
    const b2Pos posMeters{ posPixels.x / pixelsPerMeter, posPixels.y / pixelsPerMeter };
    b2Body_SetTransform(b2LoadBodyId(rawId_), posMeters, b2MakeRot(rotationRadians));
}

void PhysicsBody::setTargetTransformPixels(
    glm::vec2 posPixels,
    float rotationRadians,
    float dt,
    float pixelsPerMeter)
{
    if (!isValid() || pixelsPerMeter <= 0.0f || dt <= 0.0f) {
        return;
    }
    const b2WorldTransform target{
        b2Pos{ posPixels.x / pixelsPerMeter, posPixels.y / pixelsPerMeter },
        b2MakeRot(rotationRadians)
    };
    b2Body_SetTargetTransform(b2LoadBodyId(rawId_), target, dt, true);
}

void PhysicsBody::setLinearVelocityPixels(glm::vec2 velocityPixelsPerSec, float pixelsPerMeter) {
    if (!isValid() || pixelsPerMeter <= 0.0f) {
        return;
    }
    const b2Vec2 velMeters{
        velocityPixelsPerSec.x / pixelsPerMeter,
        velocityPixelsPerSec.y / pixelsPerMeter
    };
    b2Body_SetLinearVelocity(b2LoadBodyId(rawId_), velMeters);
}

glm::vec2 PhysicsBody::linearVelocityPixels(float pixelsPerMeter) const noexcept {
    if (!isValid() || pixelsPerMeter <= 0.0f) {
        return { 0.0f, 0.0f };
    }
    const b2Vec2 vel = b2Body_GetLinearVelocity(b2LoadBodyId(rawId_));
    return { vel.x * pixelsPerMeter, vel.y * pixelsPerMeter };
}

void PhysicsBody::setAngularVelocity(float radiansPerSec) {
    if (!isValid()) {
        return;
    }
    b2Body_SetAngularVelocity(b2LoadBodyId(rawId_), radiansPerSec);
}

float PhysicsBody::angularVelocity() const noexcept {
    if (!isValid()) {
        return 0.0f;
    }
    return b2Body_GetAngularVelocity(b2LoadBodyId(rawId_));
}

// ---------------- PhysicsWorld ----------------

struct PhysicsWorld::Impl {
    Box2DWorld world;
    float fixedDeltaTime = 1.0f / 60.0f;
    float accumulator = 0.0f;
    float maxAccumulatedTime = 0.25f;
    float pixelsPerMeter = 32.0f;
    int subStepCount = 4;
    std::vector<CollisionEvent> stepEvents;

    Impl(glm::vec2 gravity, float fixedDt, int subSteps, float ppm)
        : world(b2Vec2{ gravity.x, gravity.y })
        , fixedDeltaTime(fixedDt > 0.0f ? fixedDt : (1.0f / 60.0f))
        , pixelsPerMeter(ppm > 0.0f ? ppm : 32.0f)
        , subStepCount(std::max(1, subSteps))
    {
    }

    void dispatchStepEvents(const CollisionCallback& onCollision) {
        stepEvents.clear();
        if (!world.isValid()) {
            return;
        }

        const b2ContactEvents contactEvents = b2World_GetContactEvents(world.id());
        const b2SensorEvents sensorEvents = b2World_GetSensorEvents(world.id());

        const std::size_t estimated =
            static_cast<std::size_t>(contactEvents.beginCount) +
            static_cast<std::size_t>(contactEvents.endCount) +
            static_cast<std::size_t>(contactEvents.hitCount) +
            static_cast<std::size_t>(sensorEvents.beginCount) +
            static_cast<std::size_t>(sensorEvents.endCount);
        stepEvents.reserve(estimated);

        // Deduplicate unordered actor pairs for sensor-sensor overlaps (Box2D 3.2 emits one per sensor).
        std::vector<std::pair<Actor*, Actor*>> seenSensorBegin;
        std::vector<std::pair<Actor*, Actor*>> seenSensorEnd;

        auto makeOrderedPair = [](Actor* a, Actor* b) {
            return (a < b) ? std::pair<Actor*, Actor*>{ a, b } : std::pair<Actor*, Actor*>{ b, a };
        };

        auto emit = [&](CollisionEvent ev) {
            if (onCollision) {
                onCollision(ev);
            }
            else {
                if (ev.actorA != nullptr && !ev.actorA->isPendingDestroy()) {
                    ev.actorA->onCollision(ev);
                }
                if (ev.actorB != nullptr && ev.actorB != ev.actorA && !ev.actorB->isPendingDestroy()) {
                    CollisionEvent swapped = ev;
                    swapped.actorA = ev.actorB;
                    swapped.actorB = ev.actorA;
                    swapped.normal = -ev.normal;
                    ev.actorB->onCollision(swapped);
                }
            }
            stepEvents.push_back(std::move(ev));
        };

        for (int i = 0; i < contactEvents.beginCount; ++i) {
            const b2ContactBeginTouchEvent& e = contactEvents.beginEvents[i];
            CollisionEvent ev{};
            ev.type = CollisionEventType::BeginContact;
            ev.actorA = extractActor(e.shapeIdA);
            ev.actorB = extractActor(e.shapeIdB);
            emit(std::move(ev));
        }

        for (int i = 0; i < contactEvents.endCount; ++i) {
            const b2ContactEndTouchEvent& e = contactEvents.endEvents[i];
            CollisionEvent ev{};
            ev.type = CollisionEventType::EndContact;
            ev.actorA = extractActor(e.shapeIdA);
            ev.actorB = extractActor(e.shapeIdB);
            emit(std::move(ev));
        }

        for (int i = 0; i < contactEvents.hitCount; ++i) {
            const b2ContactHitEvent& e = contactEvents.hitEvents[i];
            CollisionEvent ev{};
            ev.type = CollisionEventType::Hit;
            ev.actorA = extractActor(e.shapeIdA);
            ev.actorB = extractActor(e.shapeIdB);
            ev.contactPoint = glm::vec2{ e.point.x, e.point.y } * pixelsPerMeter;
            ev.normal = glm::vec2{ e.normal.x, e.normal.y };
            ev.approachSpeed = e.approachSpeed;
            emit(std::move(ev));
        }

        for (int i = 0; i < sensorEvents.beginCount; ++i) {
            const b2SensorBeginTouchEvent& e = sensorEvents.beginEvents[i];
            Actor* actorA = extractActor(e.sensorShapeId);
            Actor* actorB = extractActor(e.visitorShapeId);
            if (actorA != nullptr && actorB != nullptr) {
                const auto pairKey = makeOrderedPair(actorA, actorB);
                if (std::find(seenSensorBegin.begin(), seenSensorBegin.end(), pairKey) != seenSensorBegin.end()) {
                    continue;
                }
                seenSensorBegin.push_back(pairKey);
            }

            CollisionEvent ev{};
            ev.type = CollisionEventType::SensorBegin;
            ev.actorA = actorA;
            ev.actorB = actorB;
            emit(std::move(ev));
        }

        for (int i = 0; i < sensorEvents.endCount; ++i) {
            const b2SensorEndTouchEvent& e = sensorEvents.endEvents[i];
            Actor* actorA = extractActor(e.sensorShapeId);
            Actor* actorB = extractActor(e.visitorShapeId);
            if (actorA != nullptr && actorB != nullptr) {
                const auto pairKey = makeOrderedPair(actorA, actorB);
                if (std::find(seenSensorEnd.begin(), seenSensorEnd.end(), pairKey) != seenSensorEnd.end()) {
                    continue;
                }
                seenSensorEnd.push_back(pairKey);
            }

            CollisionEvent ev{};
            ev.type = CollisionEventType::SensorEnd;
            ev.actorA = actorA;
            ev.actorB = actorB;
            emit(std::move(ev));
        }
    }
};

PhysicsWorld::PhysicsWorld(
    glm::vec2 gravityMetersPerSec2,
    float fixedDeltaTime,
    int subStepCount,
    float pixelsPerMeter)
    : impl_(std::make_unique<Impl>(gravityMetersPerSec2, fixedDeltaTime, subStepCount, pixelsPerMeter))
{
}

PhysicsWorld::~PhysicsWorld() = default;
PhysicsWorld::PhysicsWorld(PhysicsWorld&& other) noexcept = default;
PhysicsWorld& PhysicsWorld::operator=(PhysicsWorld&& other) noexcept = default;

bool PhysicsWorld::isValid() const noexcept {
    return impl_ && impl_->world.isValid();
}

void PhysicsWorld::setGravity(glm::vec2 gravityMetersPerSec2) {
    if (isValid()) {
        b2World_SetGravity(impl_->world.id(), b2Vec2{ gravityMetersPerSec2.x, gravityMetersPerSec2.y });
    }
}

glm::vec2 PhysicsWorld::gravity() const {
    if (!isValid()) {
        return { 0.0f, 0.0f };
    }
    const b2Vec2 g = b2World_GetGravity(impl_->world.id());
    return { g.x, g.y };
}

void PhysicsWorld::setPixelsPerMeter(float ppm) noexcept {
    if (impl_ && ppm > 0.0f) {
        impl_->pixelsPerMeter = ppm;
    }
}

float PhysicsWorld::pixelsPerMeter() const noexcept {
    return impl_ ? impl_->pixelsPerMeter : 32.0f;
}

glm::vec2 PhysicsWorld::toMeters(glm::vec2 pixels) const noexcept {
    return pixels / pixelsPerMeter();
}

glm::vec2 PhysicsWorld::toPixels(glm::vec2 meters) const noexcept {
    return meters * pixelsPerMeter();
}

int PhysicsWorld::update(float deltaTime, const CollisionCallback& onCollision) {
    if (!isValid() || deltaTime <= 0.0f) {
        return 0;
    }

    impl_->accumulator += std::min(deltaTime, impl_->maxAccumulatedTime);
    int steps = 0;

    while (impl_->accumulator >= impl_->fixedDeltaTime) {
        stepFixed(impl_->fixedDeltaTime, onCollision);
        impl_->accumulator -= impl_->fixedDeltaTime;
        ++steps;
    }

    return steps;
}

void PhysicsWorld::stepFixed(float dt, const CollisionCallback& onCollision) {
    if (!isValid()) {
        return;
    }

    b2World_Step(impl_->world.id(), dt, impl_->subStepCount);
    impl_->dispatchStepEvents(onCollision);
}

PhysicsBody PhysicsWorld::createBody(const BodyCreateDesc& desc) {
    if (!isValid()) {
        return PhysicsBody{};
    }

    const glm::vec2 posMeters = toMeters(desc.positionPixels);

    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = toB2BodyType(desc.type);
    bodyDef.position = b2Pos{ posMeters.x, posMeters.y };
    bodyDef.rotation = b2MakeRot(desc.rotationRadians);
    bodyDef.motionLocks.angularZ = desc.fixedRotation;
    bodyDef.isBullet = (desc.type == BodyType::Dynamic) && desc.isBullet;
    bodyDef.enableSleep = false;
    bodyDef.userData = desc.userData;

    const b2BodyId bodyId = b2CreateBody(impl_->world.id(), &bodyDef);
    if (!b2Body_IsValid(bodyId)) {
        return PhysicsBody{};
    }

    b2ShapeDef shapeDef = b2DefaultShapeDef();
    if (desc.type == BodyType::Dynamic && !desc.isSensor) {
        shapeDef.density = std::max(0.01f, desc.density);
    }
    else {
        shapeDef.density = desc.density;
    }
    shapeDef.material.friction = desc.friction;
    shapeDef.material.restitution = desc.restitution;
    shapeDef.isSensor = desc.isSensor;
    shapeDef.enableContactEvents = !desc.isSensor;
    shapeDef.enableHitEvents = !desc.isSensor;
    shapeDef.enableSensorEvents = desc.enableSensorEvents;
    shapeDef.filter.categoryBits = desc.filter.categoryBits;
    shapeDef.filter.maskBits = desc.filter.maskBits;
    shapeDef.filter.groupIndex = desc.filter.groupIndex;
    shapeDef.userData = desc.userData;

    createShapeForGeometry(bodyId, shapeDef, desc.halfSizePixels, desc.radiusPixels, impl_->pixelsPerMeter);
    return PhysicsBody{ b2StoreBodyId(bodyId) };
}

const std::vector<CollisionEvent>& PhysicsWorld::lastStepEvents() const noexcept {
    static const std::vector<CollisionEvent> kEmpty;
    return impl_ ? impl_->stepEvents : kEmpty;
}
