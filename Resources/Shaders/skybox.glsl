#pragma stage vertex
#version 450 core
layout(location = 0) in vec3 aPosition;

layout(binding = 1) uniform CameraUB
{
    mat4 uViewProjection;
    vec4 uCameraPosition;
};

layout(location = 0) out vec3 WorldPos;

void main()
{
    WorldPos = aPosition;
    vec4 pos = uViewProjection * vec4(aPosition.xyz, 1.0);
    gl_Position = pos.xyww;
}

#pragma stage end

#pragma stage fragment
#version 450 core
layout (location = 0) out vec4 aAlbedo;

layout(location = 0) in vec3 WorldPos;

layout(binding = 3) uniform samplerCube uSkybox;

void main()
{
    vec3 envColor = texture(uSkybox, WorldPos).rgb;
    envColor = pow(envColor, vec3(1.0f / 2.2));
    aAlbedo = vec4(envColor, 1.0);
}

#pragma stage end