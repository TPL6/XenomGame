#include "engine/Engine.h"

#include "GLContext.h"
#include "GLRenderer.h"
#include "SDLContext.h"
#include "Window.h"
#include "engine/InputManager.h"
#include "engine/ResourceManager.h"

#include <SDL3/SDL.h>
#include <algorithm>
#include <stdexcept>

struct Engine::Impl {
    explicit Impl(const EngineConfig& cfg)
        : config(cfg) {
        initialize();
    }

    ~Impl() {
        // Ensure active level receives onExit before RAII tears down resources and OpenGL context.
        if (activeLevel != nullptr) {
            activeLevel->onExit(resources);
            activeLevel.reset();
        }
        pendingLevel.reset();
        hasPendingLevel = false;
    }

    void initialize() {
        sdl = SDLContext(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD);
        if (!sdl.isValid()) {
            throw std::runtime_error("Failed to initialize SDL3 subsystems");
        }

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

        window = Window(config.title, config.width, config.height, SDL_WINDOW_OPENGL);
        if (!window.isValid()) {
            throw std::runtime_error("Failed to create SDL3 OpenGL window");
        }

        glContext = GLContext(window, config.vsync);
        if (!glContext.isValid()) {
            throw std::runtime_error("Failed to create OpenGL 3.3 Core context / load GLAD");
        }

        renderer = std::make_unique<GLRenderer>(window, resources, config.width, config.height);
        if (!renderer->isValid()) {
            throw std::runtime_error("Failed to initialize GLRenderer");
        }
    }

    void applyPendingLevelTransition() {
        if (!hasPendingLevel) {
            return;
        }
        hasPendingLevel = false;

        if (activeLevel != nullptr) {
            activeLevel->onExit(resources);
            activeLevel.reset();
        }

        activeLevel = std::move(pendingLevel);
        if (activeLevel != nullptr) {
            activeLevel->onEnter(resources);
        }
    }

    void fixedUpdate(float dt) {
        if (activeLevel != nullptr) {
            activeLevel->fixedUpdate(dt, input, resources);
        }
    }

    void render(float alpha) {
        if (renderer == nullptr) {
            return;
        }

        int w = config.width;
        int h = config.height;
        window.drawableSize(w, h);

        renderer->beginFrame(w, h);

        if (activeLevel != nullptr) {
            activeLevel->render(*renderer, resources, alpha);
        }

        renderer->endFrame();
    }

    EngineConfig config;
    bool running = false;

    // Declaration order controls deterministic reverse-order RAII destruction:
    // pendingLevel -> activeLevel -> input -> renderer -> resources -> glContext -> window -> sdl
    SDLContext sdl;
    Window window;
    GLContext glContext;
    ResourceManager resources;
    std::unique_ptr<GLRenderer> renderer;
    InputManager input;

    std::unique_ptr<ILevel> activeLevel;
    std::unique_ptr<ILevel> pendingLevel;
    bool hasPendingLevel = false;
};

Engine::Engine(const EngineConfig& config)
    : impl_(std::make_unique<Impl>(config)) {
}

Engine::~Engine() = default;

InputManager& Engine::input() {
    return impl_->input;
}

ResourceManager& Engine::resources() {
    return impl_->resources;
}

void Engine::setLevel(std::unique_ptr<ILevel> nextLevel) {
    impl_->pendingLevel = std::move(nextLevel);
    impl_->hasPendingLevel = true;
}

int Engine::run(std::unique_ptr<ILevel> firstLevel) {
    setLevel(std::move(firstLevel));
    return run();
}

int Engine::run() {
    impl_->running = true;
    impl_->applyPendingLevelTransition();

    Uint64 previousTicks = SDL_GetTicksNS();
    float accumulator = 0.0f;

    while (impl_->running) {
        const Uint64 currentTicks = SDL_GetTicksNS();
        float frameDelta = static_cast<float>(currentTicks - previousTicks) / 1e9f;
        previousTicks = currentTicks;

        frameDelta = std::clamp(frameDelta, 0.0f, impl_->config.maxFrameDelta);
        accumulator += frameDelta;

        // 1. Input
        impl_->input.pollEvents();
        if (impl_->input.quitRequested()) {
            impl_->running = false;
            break;
        }

        // 2. Fixed-timestep update (game logic + Box2D physics)
        while (accumulator >= impl_->config.fixedDeltaTime) {
            if (!impl_->input.isPaused()) {
                impl_->fixedUpdate(impl_->config.fixedDeltaTime);
            }
            accumulator -= impl_->config.fixedDeltaTime;
            impl_->applyPendingLevelTransition();

            if (!impl_->running) {
                break;
            }
        }

        // 3. Render (strictly separated from update)
        const float alpha = (impl_->config.fixedDeltaTime > 0.0f)
            ? std::clamp(accumulator / impl_->config.fixedDeltaTime, 0.0f, 1.0f)
            : 0.0f;

        impl_->render(alpha);
    }

    return 0;
}

void Engine::quit() {
    if (impl_) {
        impl_->running = false;
    }
}
