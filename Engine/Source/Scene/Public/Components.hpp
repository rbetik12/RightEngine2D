#pragma once

#include "Material.hpp"
#include "EnvironmentMapLoader.hpp"
#include <glm/glm.hpp>
#include <crossguid/guid.hpp>

namespace RightEngine
{
    struct TagComponent
    {
        TagComponent() = default;

        TagComponent(const std::string& name) : name(name)
        {}

        TagComponent(std::string_view name, const xg::Guid& guid) : name(name), guid(guid)
        {}

        std::string name;
        xg::Guid guid{ xg::newGuid() };
        uint32_t colorId;
    };

    // TODO: Add dirty flag, so we can recalculate transform only when needed
    struct TransformComponent
    {
    public:
        inline const glm::vec3 GetWorldPosition() const
        { return transformMatrix[3]; }

        void SetRotationDegree(const glm::vec3& newRotation)
        { rotation = glm::radians(newRotation); }

        void SetRotationRadians(const glm::vec3& newRotation)
        { rotation = newRotation; }

        glm::mat4 GetLocalTransformMatrix() const;

        const glm::mat4& GetWorldTransformMatrix() const;

        void RecalculateTransform(TransformComponent& parentTransform);

        void RecalculateTransform();

        glm::vec3 position{0.0f, 0.0f, 0.0f};
        // In radians
        glm::vec3 rotation{0.0f, 0.0f, 0.0f};
        glm::vec3 scale{1.0f, 1.0f, 1.0f};
        glm::mat4 transformMatrix{glm::mat4(1.0f)};
    };

    class MeshComponent
    {
    public:
        MeshComponent();

        AssetHandle mesh;
        AssetHandle material;
        bool isVisible;
    };

    enum class LightType
    {
        DIRECTIONAL = 0,
        POINT
    };

    struct LightComponent
    {
        LightType type{LightType::DIRECTIONAL};
        glm::vec3 color{1.0f, 1.0f, 1.0f};
        float intensity{1000.0f};

        float outerRadius{50.0f};
        float innerRadius{20.0f};
    };

    enum class SkyboxType

    {
        CUBE = 0
    };

    struct SkyboxComponent
    {
        SkyboxType type;
        AssetHandle environmentHandle;
    };

    struct CameraComponent
    {
        CameraComponent(const glm::vec3& worldUp);
        CameraComponent();
        ~CameraComponent();

        void OnUpdate(float dt);
        // Rotation must be in degrees
        glm::vec3 Rotate(float x, float y, const glm::vec3& oldRotation);
        glm::vec3 Rotate(const glm::vec3& newRotation);
        glm::vec3 Move(int keycode, const glm::vec3& oldPosition);

        glm::mat4 GetViewMatrix(const glm::vec3& position) const;
        glm::mat4 GetProjectionMatrix() const;

        inline float GetFOV(bool asRadians = false) const
        {
            if (asRadians)
            {
                return glm::radians(fov);
            }
            return fov;
        }

        glm::vec3 front;
        glm::vec3 worldUp;
        glm::vec3 up;
        float zNear{ 0.1f };
        float zFar{ 500.0f };
        float aspectRatio{ 16.0f/9.0f };
        float fov{ 50.0f };
        float movementSpeed{ 40.0f };
        float sensitivity{ 0.25f };
        float actualSpeed{ 1.0f };
        float prevXMousePos{ -1 };
        float prevYMousePos{ -1 };

        bool isActive{ false };
        bool isPrimary{ false };

    private:
        void UpdateVectors(const glm::vec3& rotation);
    };
}
