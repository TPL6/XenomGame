#version 330 core

in vec2 vTexCoord;
out vec4 FragColor;

uniform sampler2D uTexture;
uniform vec4 uTint;

void main() {
    vec4 texColor = texture(uTexture, vTexCoord);
    if (texColor.a <= 0.001 ||
        (texColor.r > 0.99 && texColor.g < 0.01 && texColor.b > 0.99)) {
        discard;
    }
    FragColor = texColor * uTint;
}
