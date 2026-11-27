#pragma once

#include "engine/Actor.h"

#include <glm/vec2.hpp>

class Level;
class InputManager;
class ResourceManager;

class Explosion final : public Actor {
public:
    Explosion(glm::vec2 pos, glm::vec2 quadSize = { 64.0f, 64.0f }, float frameDuration = 0.05f);

    void onSpawn(Level& level, ResourceManager& resources) override;
    void update(float dt, const InputManager& input, ResourceManager& resources) override;

private:
    float frameDuration_ = 0.05f;
    float spinSpeed_ = 2.0f;
};
