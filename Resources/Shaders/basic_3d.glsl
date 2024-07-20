#pragma stage vertex
#version 450

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aUv;
layout(location = 3) in vec3 aTangent;
layout(location = 4) in vec3 aBiTangent;

layout(push_constant) uniform constants
{
	mat4 cTransform;
} Transform;

layout(binding = 0) uniform CameraUB
{
    mat4 uViewProjection;
    vec4 uCameraPosition;
};

struct VertexOutput
{
    vec2 UV;
    vec3 Normal;
    vec3 WorldPos;
    mat3 TBN;
    vec4 CameraPosition;
};

layout(location = 0) out VertexOutput Output;

void main() {
    Output.UV = aUv;
    Output.Normal = mat3(Transform.cTransform) * aNormal;
    Output.WorldPos = vec3(Transform.cTransform * vec4(aPosition, 1.0));

    vec3 T = normalize(vec3(Transform.cTransform * vec4(aTangent,   0.0)));
    vec3 B = normalize(vec3(Transform.cTransform * vec4(aBiTangent, 0.0)));
    vec3 N = normalize(vec3(Transform.cTransform * vec4(aNormal,    0.0)));
    mat3 TBN = mat3(T, B, N);
    Output.TBN = TBN;
    Output.CameraPosition = uCameraPosition;

    gl_Position = uViewProjection * vec4(Output.WorldPos, 1.0);
}

#pragma stage end

#pragma stage fragment
#version 450

struct VertexOutput
{
    vec2 UV;
    vec3 Normal;
    vec3 WorldPos;
    mat3 TBN;
    vec4 CameraPosition;
};

layout(location = 0) in VertexOutput Output;

layout(location = 0) out vec4 outColor;
layout(binding = 1) uniform sampler2D u_Albedo;

struct DirectionalLight
{
    vec4	color;
    vec4	position;
    vec4	rotation;
    float	intensity;
};

struct LightBuffer
{
    DirectionalLight directionalLight;
};

layout(binding = 2) uniform LightBufferUB
{
    LightBuffer lightBuffer;
};

void main() {
    outColor = texture(u_Albedo, Output.UV);
    outColor = outColor * lightBuffer.directionalLight.color;
}

#pragma stage end