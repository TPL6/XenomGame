#pragma once

#include <SDL3/SDL.h>

class Window;

// RAII wrapper for SDL_GLContext and GLAD function loading.
class GLContext {
public:
    GLContext() = default;
    explicit GLContext(const Window& window, bool vsync = true);
    ~GLContext();

    GLContext(const GLContext&) = delete;
    GLContext& operator=(const GLContext&) = delete;

    GLContext(GLContext&& other) noexcept;
    GLContext& operator=(GLContext&& other) noexcept;

    bool isValid() const noexcept { return context_ != nullptr && gladLoaded_; }

private:
    void release() noexcept;

    SDL_GLContext context_ = nullptr;
    bool gladLoaded_ = false;
};
