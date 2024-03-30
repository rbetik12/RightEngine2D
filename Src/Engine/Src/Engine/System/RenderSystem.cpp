#include <Engine/System/RenderSystem.hpp>
#include <Engine/System/TransformSystem.hpp>
#include <Engine/Service/Window/WindowService.hpp>
#include <Engine/Registration.hpp>
#include <RHI/Pipeline.hpp>
#include <glm/gtx/euler_angles.hpp>

RTTR_REGISTRATION
{
    using namespace engine::ecs;

    engine::registration::System<engine::CameraSystem>("engine::CameraSystem")
        .UpdateAfter<engine::TransformSystem>();

    engine::registration::System<engine::RenderSystem>("engine::RenderSystem")
        .UpdateAfter<engine::CameraSystem>();

    engine::registration::Component<engine::MeshComponent>(Component::Type::ENGINE, "engine::MeshComponent");
    engine::registration::Component<engine::CameraComponent>(Component::Type::ENGINE, "engine::CameraComponent");
}

namespace
{

constexpr float         C_MOUSE_SENSITIVITY = 0.25f;
constexpr float         C_EDITOR_CAMERA_SPEED = 40.0f;
constexpr glm::vec3     C_WORLD_UP = glm::vec3(0, 1, 0);

} // unnamed

namespace engine
{

RenderSystem::RenderSystem(ecs::World* world) : System(world)
{
}

void RenderSystem::Update(float dt)
{
    PROFILER_CPU_ZONE;

    auto& rs = Instance().Service<RenderService>();

    rs.BeginPass(rs.DefaultPipeline());
    
    for (auto [e, mesh] : W()->View<MeshComponent>().each())
    {
        for (const auto& submesh : mesh.m_mesh->GetSubMeshList())
        {
            ENGINE_ASSERT(mesh.m_material);
            rs.BindMaterial(mesh.m_material, rs.DefaultPipeline());
            rs.Draw(submesh->VertexBuffer(), rs.DefaultPipeline()->VertexCount(submesh->VertexBuffer()));
        }
    }
    
    rs.EndPass(rs.DefaultPipeline());
}

CameraSystem::CameraSystem(ecs::World* world) : System(world)
{
}

void CameraSystem::Update(float dt)
{
    PROFILER_CPU_ZONE;

    auto& windowService = Instance().Service<WindowService>();

    for (auto [e, c, t] : W()->View<CameraComponent, TransformComponent>().each())
    {
        if (!c.m_active)
        {
            continue;
        }

        if (c.m_type == CameraComponent::Type::EDITOR)
        {
            const float actualSpeed = C_EDITOR_CAMERA_SPEED * dt;

            if (windowService.KeyButtonPressed(KeyButton::KEY_W))
            {
                t.m_position += c.m_front * actualSpeed;
            }
            if (windowService.KeyButtonPressed(KeyButton::KEY_S))
            {
                t.m_position -= c.m_front * actualSpeed;
            }
            if (windowService.KeyButtonPressed(KeyButton::KEY_A))
            {
                t.m_position -= glm::normalize(glm::cross(c.m_front, c.m_up)) * actualSpeed;
            }
            if (windowService.KeyButtonPressed(KeyButton::KEY_D))
            {
                t.m_position += glm::normalize(glm::cross(c.m_front, c.m_up)) * actualSpeed;
            }

            t.Reset();
            t.Modify();
        }

        const auto prevMousePos = windowService.PrevMousePos();
        const auto mousePos = windowService.MousePos();

        if (c.IsRecentlyCreated() || prevMousePos != mousePos)
        {
            float xOffset = mousePos.x - prevMousePos.x;
            float yOffset = prevMousePos.y - mousePos.y;
            xOffset *= C_MOUSE_SENSITIVITY;
            yOffset *= C_MOUSE_SENSITIVITY;

            // in radians
            auto rotation = glm::eulerAngles(t.m_rotation);

            rotation[0] += xOffset;
            rotation[1] += yOffset;

            rotation[1] = glm::degrees(rotation[1]);

            if (rotation[1] > 89.0f)
            {
                rotation[1] = 89.0f;
            }
            if (rotation[1] < -89.0f)
            {
                rotation[1] = -89.0f;
            }

            rotation[1] = glm::radians(rotation[1]);

            const float yaw = rotation[0];
            const float pitch = rotation[1];

            glm::vec3 rotatedFront;
            rotatedFront.x = cos(yaw) * cos(pitch);
            rotatedFront.y = sin(pitch);
            rotatedFront.z = sin(yaw) * cos(pitch);

            c.m_front = glm::normalize(rotatedFront);
            const auto right = glm::normalize(glm::cross(c.m_front, C_WORLD_UP));
            c.m_up = glm::normalize(glm::cross(right, c.m_front));

            c.Reset();
            c.Modify();

            if (c.IsRecentlyCreated())
            {
                c.Init();
            }
        }

        c.m_view = glm::lookAt(t.m_position, t.m_position + c.m_front, c.m_up);
        c.m_proj = glm::perspective(glm::radians(c.m_fov), c.m_aspectRatio, c.m_near, c.m_far);
        c.m_projView = c.m_proj * c.m_view;
    }
}

} // engine
