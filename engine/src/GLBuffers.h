#pragma once

#include <cstddef>

// RAII wrapper for an OpenGL Vertex Array Object (VAO).
class VertexArray {
public:
    VertexArray() = default;
    ~VertexArray();

    VertexArray(const VertexArray&) = delete;
    VertexArray& operator=(const VertexArray&) = delete;

    VertexArray(VertexArray&& other) noexcept;
    VertexArray& operator=(VertexArray&& other) noexcept;

    static VertexArray create();

    bool isValid() const noexcept { return id_ != 0; }
    unsigned int id() const noexcept { return id_; }

    void bind() const noexcept;
    static void unbind() noexcept;

private:
    explicit VertexArray(unsigned int id) noexcept : id_(id) {}
    void release() noexcept;

    unsigned int id_ = 0;
};

// RAII wrapper for an OpenGL Vertex Buffer Object (VBO).
class VertexBuffer {
public:
    VertexBuffer() = default;
    ~VertexBuffer();

    VertexBuffer(const VertexBuffer&) = delete;
    VertexBuffer& operator=(const VertexBuffer&) = delete;

    VertexBuffer(VertexBuffer&& other) noexcept;
    VertexBuffer& operator=(VertexBuffer&& other) noexcept;

    static VertexBuffer create(const void* data, std::size_t sizeBytes);

    bool isValid() const noexcept { return id_ != 0; }
    unsigned int id() const noexcept { return id_; }

    void bind() const noexcept;
    static void unbind() noexcept;

private:
    explicit VertexBuffer(unsigned int id) noexcept : id_(id) {}
    void release() noexcept;

    unsigned int id_ = 0;
};

// RAII wrapper for an OpenGL Element Buffer Object (EBO / IBO).
class IndexBuffer {
public:
    IndexBuffer() = default;
    ~IndexBuffer();

    IndexBuffer(const IndexBuffer&) = delete;
    IndexBuffer& operator=(const IndexBuffer&) = delete;

    IndexBuffer(IndexBuffer&& other) noexcept;
    IndexBuffer& operator=(IndexBuffer&& other) noexcept;

    static IndexBuffer create(const unsigned int* indices, std::size_t count);

    bool isValid() const noexcept { return id_ != 0; }
    unsigned int id() const noexcept { return id_; }
    std::size_t count() const noexcept { return count_; }

    void bind() const noexcept;
    static void unbind() noexcept;

private:
    IndexBuffer(unsigned int id, std::size_t count) noexcept : id_(id), count_(count) {}
    void release() noexcept;

    unsigned int id_ = 0;
    std::size_t count_ = 0;
};
