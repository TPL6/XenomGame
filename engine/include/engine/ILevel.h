#pragma once

class InputManager;
class Renderer;
class ResourceManager;

class ILevel {
public:
    virtual ~ILevel() = default;

    virtual void onEnter(ResourceManager& resources) = 0;
    virtual void onExit(ResourceManager& resources) = 0;
    virtual void fixedUpdate(float dt, const InputManager& input, ResourceManager& resources) = 0;
    virtual void render(Renderer& renderer, ResourceManager& resources, float alpha) = 0;
};
