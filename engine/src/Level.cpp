#include "engine/Level.h"

#include "engine/InputManager.h"
#include "engine/ResourceManager.h"

#include <algorithm>
#include <cstddef>

Level::Level(
    glm::vec2 gravityMetersPerSec2,
    float fixedDeltaTime,
    int subStepCount,
    float pixelsPerMeter)
    : physicsWorld_(gravityMetersPerSec2, fixedDeltaTime, subStepCount, pixelsPerMeter)
{
}

Level::~Level() {
    clearActors();
}

void Level::queueDestroyActor(Actor* actor) {
    if (actor == nullptr) {
        return;
    }
    actor->destroy();
    pendingDestroyIds_.push_back(actor->id());
}

void Level::queueDestroyActorById(Actor::ActorId id) {
    if (id == 0) {
        return;
    }
    if (Actor* found = findActorById(id); found != nullptr) {
        found->destroy();
    }
    pendingDestroyIds_.push_back(id);
}

Actor* Level::findActorById(Actor::ActorId id) const {
    if (id == 0) {
        return nullptr;
    }

    for (const auto& actor : actors_) {
        if (actor && actor->id() == id && !actor->isPendingDestroy()) {
            return actor.get();
        }
    }

    for (const auto& actor : pendingSpawns_) {
        if (actor && actor->id() == id && !actor->isPendingDestroy()) {
            return actor.get();
        }
    }

    return nullptr;
}

void Level::addParallaxLayer(const ParallaxLayerDesc& layer) {
    parallaxLayers_.push_back(layer);
}

void Level::clearParallaxLayers() {
    parallaxLayers_.clear();
}

void Level::onEnter(ResourceManager& resources) {
    flushSpawns(resources);
}

void Level::onExit(ResourceManager& /*resources*/) {
    clearActors();
    parallaxLayers_.clear();
}

void Level::flushSpawns(ResourceManager& resources) {
    while (!pendingSpawns_.empty()) {
        std::vector<std::unique_ptr<Actor>> newlySpawned = std::move(pendingSpawns_);
        pendingSpawns_.clear();

        for (auto& actor : newlySpawned) {
            if (!actor) {
                continue;
            }
            actor->id_ = nextActorId_++;
            actor->level_ = this;
            actor->onSpawn(*this, resources);
            actors_.push_back(std::move(actor));
        }
    }
}

void Level::flushDestroys() {
    bool anyMarked = !pendingDestroyIds_.empty();
    if (!anyMarked) {
        for (const auto& actor : actors_) {
            if (actor && actor->isPendingDestroy()) {
                anyMarked = true;
                break;
            }
        }
    }

    if (!anyMarked) {
        return;
    }

    auto shouldRemove = [&](const std::unique_ptr<Actor>& actor) -> bool {
        if (!actor) {
            return true;
        }
        if (actor->isPendingDestroy()) {
            return true;
        }
        return std::find(pendingDestroyIds_.begin(), pendingDestroyIds_.end(), actor->id())
            != pendingDestroyIds_.end();
    };

    for (auto& actor : actors_) {
        if (shouldRemove(actor) && actor) {
            actor->onDespawn();
            actor->body().reset();
            actor->level_ = nullptr;
        }
    }

    actors_.erase(
        std::remove_if(actors_.begin(), actors_.end(), shouldRemove),
        actors_.end()
    );

    pendingDestroyIds_.clear();
}

void Level::clearActors() {
    for (auto& actor : actors_) {
        if (actor) {
            actor->onDespawn();
            actor->body().reset();
            actor->level_ = nullptr;
        }
    }
    actors_.clear();

    for (auto& actor : pendingSpawns_) {
        if (actor) {
            actor->onDespawn();
            actor->body().reset();
            actor->level_ = nullptr;
        }
    }
    pendingSpawns_.clear();
    pendingDestroyIds_.clear();
}

void Level::fixedUpdate(float dt, const InputManager& input, ResourceManager& resources) {
    flushSpawns(resources);

    // 1. Update gameplay logic & animations for all actors
    for (const auto& actor : actors_) {
        if (actor && !actor->isPendingDestroy()) {
            actor->update(dt, input, resources);
        }
    }

    // Spawn any actors created during update (e.g., missiles, enemy shots) before physics step
    flushSpawns(resources);

    // 2. Sync kinematic/static bodies -> step Box2D -> sync dynamic bodies
    if (physicsWorld_.isValid()) {
        const float ppm = physicsWorld_.pixelsPerMeter();

        for (const auto& actor : actors_) {
            if (actor && !actor->isPendingDestroy() && actor->hasBody()) {
                const BodyType type = actor->body().type();
                if (type == BodyType::Kinematic) {
                    actor->syncTargetToPhysics(dt, ppm);
                }
                else if (type == BodyType::Static) {
                    actor->syncToPhysics(ppm);
                }
            }
        }

        physicsWorld_.stepFixed(dt);

        for (const auto& actor : actors_) {
            if (actor && !actor->isPendingDestroy() && actor->hasBody() &&
                actor->body().type() == BodyType::Dynamic) {
                actor->syncFromPhysics(ppm);
            }
        }
    }

    // 3. Flush spawns created in collision callbacks (e.g., explosions) and destroy dead actors
    flushSpawns(resources);
    flushDestroys();
}

void Level::render(Renderer& renderer, ResourceManager& /*resources*/, float /*alpha*/) {
    for (const auto& layer : parallaxLayers_) {
        renderer.drawParallaxLayer(layer);
    }

    // Sort indices by drawOrder without mutating actor ownership order
    std::vector<std::size_t> indices(actors_.size());
    for (std::size_t i = 0; i < actors_.size(); ++i) {
        indices[i] = i;
    }

    std::stable_sort(indices.begin(), indices.end(), [&](std::size_t a, std::size_t b) {
        const int orderA = actors_[a] ? actors_[a]->drawOrder_ : 0;
        const int orderB = actors_[b] ? actors_[b]->drawOrder_ : 0;
        return orderA < orderB;
    });

    for (std::size_t idx : indices) {
        const auto& actor = actors_[idx];
        if (actor && !actor->isPendingDestroy()) {
            actor->render(renderer);
        }
    }
}
