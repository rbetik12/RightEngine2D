#pragma stage vertex
#version 450

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec2 aUV;

layout(location = 0) out vec2 uv;

void main() {
    gl_Position = vec4(aPosition, 1.0);
    uv = aUV;
}

#pragma stage end

#pragma stage fragment
#version 450

layout(location = 0) in vec2 uv;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform usampler2D uTexture;

void main() {
    outColor = texture(uTexture, uv);
}

#pragma stage end