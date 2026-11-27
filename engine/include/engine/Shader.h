#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <string>
#include <string_view>
#include <unordered_map>

class Shader {
public:
    Shader() = default;
    Shader(std::string_view vertexPath, std::string_view fragmentPath);
    ~Shader();

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    Shader(Shader&& other) noexcept;
    Shader& operator=(Shader&& other) noexcept;

    static Shader fromSource(
        std::string_view vertexSource,
        std::string_view fragmentSource,
        std::string_view label = "inline"
    );

    bool isValid() const noexcept { return programId_ != 0; }
    unsigned int id() const noexcept { return programId_; }

    void bind() const;
    static void unbind();

    void setInt(std::string_view name, int value) const;
    void setFloat(std::string_view name, float value) const;
    void setVec2(std::string_view name, const glm::vec2& value) const;
    void setVec3(std::string_view name, const glm::vec3& value) const;
    void setVec4(std::string_view name, const glm::vec4& value) const;
    void setMat4(std::string_view name, const glm::mat4& value) const;

private:
    bool buildFromSources(
        const std::string& vertexSource,
        const std::string& fragmentSource,
        std::string_view vertexLabel,
        std::string_view fragmentLabel
    );
    static std::string loadFile(std::string_view path);
    static unsigned int compileShader(unsigned int type, const std::string& source, std::string_view label);
    int getUniformLocation(std::string_view name) const;
    void release();

    unsigned int programId_ = 0;
    mutable std::unordered_map<std::string, int> uniformCache_;
};
