#pragma once

#include "engine/Level.h"
#include "engine/Renderer.h"

#include <glm/vec2.hpp>
#include <random>

class InputManager;
class ResourceManager;

class VerticalXenonLevel final : public Level {
public:
    VerticalXenonLevel(float width = 640.0f, float height = 480.0f);
    ~VerticalXenonLevel() override = default;

    void onEnter(ResourceManager& resources) override;
    void onExit(ResourceManager& resources) override;

    void fixedUpdate(float dt, const InputManager& input, ResourceManager& resources) override;
    void render(Renderer& renderer, ResourceManager& resources, float alpha) override;

private:
    void setupParallaxLayers(ResourceManager& resources);
    void updateParallaxScroll(float dt);
    void updatePlayerRespawn(float dt);
    void updateWaves(float dt);
    void spawnLonerWave();
    void spawnRusherWave();

    float width_ = 640.0f;
    float height_ = 480.0f;

    Actor::ActorId playerId_ = 0;
    float respawnTimer_ = 0.0f;

    TextureHandle farBgHandle_ = kInvalidTextureHandle;
    glm::vec2 farScroll_{ 0.0f, 0.0f };
    float farScrollSpeed_ = 0.06f;
    int farLayerIdx_ = -1;

    float waveTimer_ = 0.0f;
    int waveIndex_ = 0;
    std::mt19937 rng_;
};
