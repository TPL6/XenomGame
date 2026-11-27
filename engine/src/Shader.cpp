#include "engine/Shader.h"

#include "AssetPath.h"

#include <cstddef>
#include <fstream>
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <sstream>
#include <utility>

Shader::Shader(std::string_view vertexPath, std::string_view fragmentPath) {
    const std::string vertSource = loadFile(vertexPath);
    const std::string fragSource = loadFile(fragmentPath);
    if (vertSource.empty() || fragSource.empty()) {
        return;
    }

    buildFromSources(vertSource, fragSource, vertexPath, fragmentPath);
}

Shader Shader::fromSource(
    std::string_view vertexSource,
    std::string_view fragmentSource,
    std::string_view label)
{
    Shader shader;
    shader.buildFromSources(
        std::string(vertexSource),
        std::string(fragmentSource),
        std::string(label) + ":vertex",
        std::string(label) + ":fragment"
    );
    return shader;
}

bool Shader::buildFromSources(
    const std::string& vertexSource,
    const std::string& fragmentSource,
    std::string_view vertexLabel,
    std::string_view fragmentLabel)
{
    const unsigned int vertShader = compileShader(GL_VERTEX_SHADER, vertexSource, vertexLabel);
    if (vertShader == 0) {
        return false;
    }

    const unsigned int fragShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource, fragmentLabel);
    if (fragShader == 0) {
        glDeleteShader(vertShader);
        return false;
    }

    const unsigned int program = glCreateProgram();
    glAttachShader(program, vertShader);
    glAttachShader(program, fragShader);
    glLinkProgram(program);

    int linked = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (!linked) {
        int logLength = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
        std::string infoLog(static_cast<std::size_t>(logLength > 0 ? logLength : 1), '\0');
        glGetProgramInfoLog(program, logLength, nullptr, infoLog.data());
        std::cerr << "Shader program link error (" << vertexLabel << ", " << fragmentLabel << "):\n"
                  << infoLog << "\n";
        glDeleteProgram(program);
        glDeleteShader(vertShader);
        glDeleteShader(fragShader);
        return false;
    }

    glDetachShader(program, vertShader);
    glDetachShader(program, fragShader);
    glDeleteShader(vertShader);
    glDeleteShader(fragShader);

    programId_ = program;
    return true;
}

Shader::~Shader() {
    release();
}

Shader::Shader(Shader&& other) noexcept
    : programId_(other.programId_)
    , uniformCache_(std::move(other.uniformCache_)) {
    other.programId_ = 0;
}

Shader& Shader::operator=(Shader&& other) noexcept {
    if (this != &other) {
        release();
        programId_ = other.programId_;
        uniformCache_ = std::move(other.uniformCache_);
        other.programId_ = 0;
    }
    return *this;
}

void Shader::release() {
    if (programId_ != 0) {
        glDeleteProgram(programId_);
        programId_ = 0;
    }
    uniformCache_.clear();
}

void Shader::bind() const {
    glUseProgram(programId_);
}

void Shader::unbind() {
    glUseProgram(0);
}

std::string Shader::loadFile(std::string_view path) {
    const std::string resolvedPath = resolveAssetPath(path);
    std::ifstream file(resolvedPath, std::ios::in | std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open shader file: " << path << "\n";
        return {};
    }
    std::ostringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

unsigned int Shader::compileShader(unsigned int type, const std::string& source, std::string_view label) {
    const unsigned int shader = glCreateShader(type);
    const char* srcPtr = source.c_str();
    glShaderSource(shader, 1, &srcPtr, nullptr);
    glCompileShader(shader);

    int compiled = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        int logLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
        std::string infoLog(static_cast<std::size_t>(logLength > 0 ? logLength : 1), '\0');
        glGetShaderInfoLog(shader, logLength, nullptr, infoLog.data());
        std::cerr << "Shader compile error (" << label << "):\n"
                  << infoLog << "\n";
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

int Shader::getUniformLocation(std::string_view name) const {
    const std::string key(name);
    if (const auto it = uniformCache_.find(key); it != uniformCache_.end()) {
        return it->second;
    }
    const int location = glGetUniformLocation(programId_, key.c_str());
    uniformCache_.emplace(key, location);
    return location;
}

void Shader::setInt(std::string_view name, int value) const {
    const int loc = getUniformLocation(name);
    if (loc != -1) {
        glUniform1i(loc, value);
    }
}

void Shader::setFloat(std::string_view name, float value) const {
    const int loc = getUniformLocation(name);
    if (loc != -1) {
        glUniform1f(loc, value);
    }
}

void Shader::setVec2(std::string_view name, const glm::vec2& value) const {
    const int loc = getUniformLocation(name);
    if (loc != -1) {
        glUniform2fv(loc, 1, glm::value_ptr(value));
    }
}

void Shader::setVec3(std::string_view name, const glm::vec3& value) const {
    const int loc = getUniformLocation(name);
    if (loc != -1) {
        glUniform3fv(loc, 1, glm::value_ptr(value));
    }
}

void Shader::setVec4(std::string_view name, const glm::vec4& value) const {
    const int loc = getUniformLocation(name);
    if (loc != -1) {
        glUniform4fv(loc, 1, glm::value_ptr(value));
    }
}

void Shader::setMat4(std::string_view name, const glm::mat4& value) const {
    const int loc = getUniformLocation(name);
    if (loc != -1) {
        glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(value));
    }
}
