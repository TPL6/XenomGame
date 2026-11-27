#pragma once

#include <SDL3/SDL.h>
#include <string_view>

// RAII wrapper for SDL_Window.
class Window {
public:
    Window() = default;
    Window(std::string_view title, int width, int height, SDL_WindowFlags flags);
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    Window(Window&& other) noexcept;
    Window& operator=(Window&& other) noexcept;

    bool isValid() const noexcept { return handle_ != nullptr; }
    SDL_Window* nativeHandle() const noexcept { return handle_; }

    void drawableSize(int& outWidth, int& outHeight) const noexcept;
    void swapBuffers() const noexcept;

private:
    void release() noexcept;

    SDL_Window* handle_ = nullptr;
};
