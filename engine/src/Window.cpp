#include "Window.h"

#include <iostream>
#include <string>

Window::Window(std::string_view title, int width, int height, SDL_WindowFlags flags) {
    const std::string titleStr(title);
    handle_ = SDL_CreateWindow(titleStr.c_str(), width, height, flags);
    if (handle_ == nullptr) {
        std::cerr << "Window: SDL_CreateWindow failed: " << SDL_GetError() << "\n";
    }
}

Window::~Window() {
    release();
}

Window::Window(Window&& other) noexcept
    : handle_(other.handle_) {
    other.handle_ = nullptr;
}

Window& Window::operator=(Window&& other) noexcept {
    if (this != &other) {
        release();
        handle_ = other.handle_;
        other.handle_ = nullptr;
    }
    return *this;
}

void Window::drawableSize(int& outWidth, int& outHeight) const noexcept {
    if (handle_ != nullptr) {
        SDL_GetWindowSizeInPixels(handle_, &outWidth, &outHeight);
    }
}

void Window::swapBuffers() const noexcept {
    if (handle_ != nullptr) {
        SDL_GL_SwapWindow(handle_);
    }
}

void Window::release() noexcept {
    if (handle_ != nullptr) {
        SDL_DestroyWindow(handle_);
        handle_ = nullptr;
    }
}
