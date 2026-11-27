#pragma once

#include "engine/Engine.h"
#include "engine/ILevel.h"

#include <memory>

class Game {
public:
    virtual ~Game() = default;

    // Start-up settings. Called BEFORE the engine is created so it can configure the window.
    virtual EngineConfig config() const { return {}; }

    // Called once the engine is initialized, before the first level enters.
    virtual void setup(Engine& /*engine*/) {}

    // Creates the initial level for the game loop. The engine takes ownership.
    virtual std::unique_ptr<ILevel> createFirstLevel() = 0;
};

// Declared by the engine, defined by the game executable, called by the engine's EntryPoint.cpp.
std::unique_ptr<Game> createGame();
