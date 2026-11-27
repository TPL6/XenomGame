#pragma once

#include "engine/ILevel.h"

#include <memory>
#include <string>

class InputManager;
class ResourceManager;

struct EngineConfig {
    std::string title = "2D Game Engine";
    int width = 640;
    int height = 480;
    bool vsync = true;
    float maxFrameDelta = 0.25f;
    float fixedDeltaTime = 1.0f / 60.0f;
};

class Engine {
public:
    explicit Engine(const EngineConfig& config = {});
    ~Engine();

    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;

    InputManager& input();
    ResourceManager& resources();

    void setLevel(std::unique_ptr<ILevel> nextLevel);
    int run();
    int run(std::unique_ptr<ILevel> firstLevel);
    void quit();

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
