#version 330 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

uniform mat4 uProjection;
uniform mat4 uModel;
uniform vec4 uUVRect;
uniform vec2 uUVScroll;
uniform vec2 uUVScale;

out vec2 vTexCoord;

void main() {
    vec2 subUV = mix(uUVRect.xy, uUVRect.zw, aTexCoord);
    vTexCoord = subUV * uUVScale + uUVScroll;
    gl_Position = uProjection * uModel * vec4(aPos, 0.0, 1.0);
}
