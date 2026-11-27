#pragma once

#include "engine/Actor.h"
#include "engine/ILevel.h"
#include "engine/PhysicsWorld.h"
#include "engine/Renderer.h"

#include <cstdint>
#include <glm/vec2.hpp>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

class InputManager;
class ResourceManager;

class Level : public ILevel {
public:
    explicit Level(
        glm::vec2 gravityMetersPerSec2 = { 0.0f, 0.0f },
        float fixedDeltaTime = 1.0f / 60.0f,
        int subStepCount = 4,
        float pixelsPerMeter = 32.0f
    );
    ~Level() override;

    Level(const Level&) = delete;
    Level& operator=(const Level&) = delete;

    PhysicsWorld& physics() noexcept { return physicsWorld_; }
    const PhysicsWorld& physics() const noexcept { return physicsWorld_; }

    template <typename TActor, typename... Args>
    TActor* spawnActor(Args&&... args) {
        static_assert(std::is_base_of_v<Actor, TActor>, "TActor must derive from Actor");
        auto actor = std::make_unique<TActor>(std::forward<Args>(args)...);
        TActor* rawPtr = actor.get();
        pendingSpawns_.push_back(std::move(actor));
        return rawPtr;
    }

    void queueDestroyActor(Actor* actor);
    void queueDestroyActorById(Actor::ActorId id);
    void clearActors();

    Actor* findActorById(Actor::ActorId id) const;
    const std::vector<std::unique_ptr<Actor>>& actors() const noexcept { return actors_; }

    void addParallaxLayer(const ParallaxLayerDesc& layer);
    void clearParallaxLayers();
    std::vector<ParallaxLayerDesc>& parallaxLayers() { return parallaxLayers_; }

    void onEnter(ResourceManager& resources) override;
    void onExit(ResourceManager& resources) override;
    void fixedUpdate(float dt, const InputManager& input, ResourceManager& resources) override;
    void render(Renderer& renderer, ResourceManager& resources, float alpha) override;

protected:
    void flushSpawns(ResourceManager& resources);
    void flushDestroys();

private:
    // Declared FIRST so it is destroyed LAST (after all actors and their PhysicsBody members).
    PhysicsWorld physicsWorld_;

    Actor::ActorId nextActorId_ = 1;
    std::vector<std::unique_ptr<Actor>> actors_;
    std::vector<std::unique_ptr<Actor>> pendingSpawns_;
    std::vector<Actor::ActorId> pendingDestroyIds_;
    std::vector<ParallaxLayerDesc> parallaxLayers_;
};
