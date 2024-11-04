#pragma once

#include <Engine/Config.hpp>
#include <Engine/ECS/System.hpp>
#include <Engine/ECS/World.hpp>
#include <Engine/ECS/Component.hpp>
#include <Engine/Service/Resource/MeshResource.hpp>

namespace engine
{

struct PBRMaterialUB
{
    // Default value is red plastic
    glm::vec3 m_albedoVec = glm::vec3(1.0f, 0.0f, 0.0f);

    float m_roughness = 1.0f;
    float m_metallic = 0.0f;

    bool m_useAlbedoTex = false;
    bool m_useNormalTex = false;
    bool m_useMetallicTex = false;
    bool m_useRoughnessTex = false;

    glm::vec2 _padding_;
};

struct CameraUB
{
    glm::mat4 m_projView;
    glm::vec4 m_position;
};

struct LightBufferUB
{
    struct DirectionalLight
    {
        glm::vec4	m_color;
        glm::vec4	m_position;
        glm::vec4	m_rotation;
        float	    m_intensity = 0.0f;
    };

    DirectionalLight m_directionalLight;
};

struct ENGINE_API DirectionalLightComponent : public ecs::Component
{
    glm::vec3   m_color = glm::vec3(1.0f);
    float       m_intensity = 1000.0f;
};

struct ENGINE_API MeshComponent : public ecs::Component
{
    std::shared_ptr<MaterialResource>    m_material;
    std::shared_ptr<MeshResource>        m_mesh;
};

class ENGINE_API RenderSystem : public ecs::System<RenderSystem>
{
public:
    RenderSystem(ecs::World* world);
    virtual ~RenderSystem() = default;

    virtual void Update(float dt) override;
};

struct ENGINE_API CameraComponent : public ecs::Component
{
    enum class Type : uint8_t
    {
        EDITOR = 0,
        GAME
    };

    glm::mat4	m_view = glm::mat4(0);
    glm::mat4	m_proj = glm::mat4(0);
    glm::mat4	m_projView = glm::mat4(0);

    glm::vec3   m_front = glm::vec3(0.0f);
    glm::vec3   m_up = glm::vec3(0.0f);

    float       m_near = 0.1f ;
    float       m_far = 5000.0f;
    float       m_aspectRatio = 16.0f / 9.0f;
    float       m_fov = glm::radians(70.0f);

    Type        m_type = Type::EDITOR;
    bool        m_active = false;
};

class ENGINE_API CameraSystem : public ecs::System<CameraSystem>
{
public:
    CameraSystem(ecs::World* world);
    virtual ~CameraSystem() = default;

    virtual void Update(float dt) override;
};

} // engine