#include "GLBuffers.h"

#include <glad/glad.h>

// ---------------- VertexArray ----------------

VertexArray VertexArray::create() {
    unsigned int id = 0;
    glGenVertexArrays(1, &id);
    return VertexArray(id);
}

VertexArray::~VertexArray() {
    release();
}

VertexArray::VertexArray(VertexArray&& other) noexcept
    : id_(other.id_) {
    other.id_ = 0;
}

VertexArray& VertexArray::operator=(VertexArray&& other) noexcept {
    if (this != &other) {
        release();
        id_ = other.id_;
        other.id_ = 0;
    }
    return *this;
}

void VertexArray::bind() const noexcept {
    glBindVertexArray(id_);
}

void VertexArray::unbind() noexcept {
    glBindVertexArray(0);
}

void VertexArray::release() noexcept {
    if (id_ != 0) {
        glDeleteVertexArrays(1, &id_);
        id_ = 0;
    }
}

// ---------------- VertexBuffer ----------------

VertexBuffer VertexBuffer::create(const void* data, std::size_t sizeBytes) {
    unsigned int id = 0;
    glGenBuffers(1, &id);
    glBindBuffer(GL_ARRAY_BUFFER, id);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(sizeBytes), data, GL_STATIC_DRAW);
    return VertexBuffer(id);
}

VertexBuffer::~VertexBuffer() {
    release();
}

VertexBuffer::VertexBuffer(VertexBuffer&& other) noexcept
    : id_(other.id_) {
    other.id_ = 0;
}

VertexBuffer& VertexBuffer::operator=(VertexBuffer&& other) noexcept {
    if (this != &other) {
        release();
        id_ = other.id_;
        other.id_ = 0;
    }
    return *this;
}

void VertexBuffer::bind() const noexcept {
    glBindBuffer(GL_ARRAY_BUFFER, id_);
}

void VertexBuffer::unbind() noexcept {
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VertexBuffer::release() noexcept {
    if (id_ != 0) {
        glDeleteBuffers(1, &id_);
        id_ = 0;
    }
}

// ---------------- IndexBuffer ----------------

IndexBuffer IndexBuffer::create(const unsigned int* indices, std::size_t count) {
    unsigned int id = 0;
    glGenBuffers(1, &id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(count * sizeof(unsigned int)),
        indices,
        GL_STATIC_DRAW
    );
    return IndexBuffer(id, count);
}

IndexBuffer::~IndexBuffer() {
    release();
}

IndexBuffer::IndexBuffer(IndexBuffer&& other) noexcept
    : id_(other.id_)
    , count_(other.count_) {
    other.id_ = 0;
    other.count_ = 0;
}

IndexBuffer& IndexBuffer::operator=(IndexBuffer&& other) noexcept {
    if (this != &other) {
        release();
        id_ = other.id_;
        count_ = other.count_;
        other.id_ = 0;
        other.count_ = 0;
    }
    return *this;
}

void IndexBuffer::bind() const noexcept {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id_);
}

void IndexBuffer::unbind() noexcept {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void IndexBuffer::release() noexcept {
    if (id_ != 0) {
        glDeleteBuffers(1, &id_);
        id_ = 0;
    }
    count_ = 0;
}
