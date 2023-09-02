#include "Components.hpp"
#include "MeshBuilder.hpp"
#include "KeyCodes.hpp"
#include "AssetManager.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>

using namespace RightEngine;

// Mesh-----------------------------------------------------------------------------------------------------------------------------------------------
MeshComponent::MeshComponent()
{
    auto& assetManager = AssetManager::Get();
    material = assetManager.GetDefaultMaterial();
    // TODO: Remove that temporary cube mesh creation in favor of proper asset management with content panel
    mesh = MeshBuilder::Cube();
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

// Camera---------------------------------------------------------------------------------------------------------------------------------------------
CameraComponent::CameraComponent(const glm::vec3& worldUp) : worldUp(worldUp)
{
    UpdateVectors(glm::vec3(0));
}

CameraComponent::CameraComponent() : worldUp(glm::vec3(0, 1, 0))
{
    UpdateVectors(glm::vec3(0));
}

CameraComponent::~CameraComponent()
{
}

void CameraComponent::OnUpdate(float dt)
{
    actualSpeed = movementSpeed * dt;
}

glm::mat4 CameraComponent::GetViewMatrix(const glm::vec3& position) const
{
    return glm::lookAt(position, position + front, up);
}

glm::mat4 CameraComponent::GetProjectionMatrix() const
{
    return glm::perspective(glm::radians(fov), aspectRatio, zNear, zFar);
}

void CameraComponent::UpdateVectors(const glm::vec3& rotation)
{
    float yaw = rotation[0];
    float pitch = rotation[1];
    glm::vec3 rotatedFront;
    rotatedFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    rotatedFront.y = sin(glm::radians(pitch));
    rotatedFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

    front = glm::normalize(rotatedFront);
    const glm::vec3 right = glm::normalize(glm::cross(front, worldUp));
    up = glm::normalize(glm::cross(right, front));
}

glm::vec3 CameraComponent::Rotate(float x, float y, const glm::vec3& oldRotation)
{
    glm::vec3 rotation = oldRotation;
    if (!isActive)
    {
        prevXMousePos = x;
        prevYMousePos = y;
        return rotation;
    }
    if (prevXMousePos == -1)
    {
        prevXMousePos = x;
        prevYMousePos = y;
    }
    float xOffset = x - prevXMousePos;
    float yOffset = prevYMousePos - y;
    xOffset *= sensitivity;
    yOffset *= sensitivity;

    rotation[0] += xOffset;
    rotation[1] += yOffset;

    if (rotation[1] > 89.0f)
    {
        rotation[1] = 89.0f;
    }
    if (rotation[1] < -89.0f)
    {
        rotation[1] = -89.0f;
    }
    UpdateVectors(rotation);
    prevXMousePos = x;
    prevYMousePos = y;
    return rotation;
}

glm::vec3 CameraComponent::Rotate(const glm::vec3& newRotation)
{
    UpdateVectors(newRotation);
    return newRotation;
}

glm::vec3 CameraComponent::Move(int keycode, const glm::vec3& oldPosition)
{
    glm::vec3 position = oldPosition;
    if (keycode == R_KEY_W)
    {
        position += actualSpeed * front;
    }
    if (keycode == R_KEY_S)
    {
        position -= actualSpeed * front;
    }
    if (keycode == R_KEY_A)
    {
        position -= glm::normalize(glm::cross(front, up)) * actualSpeed;
    }
    if (keycode == R_KEY_D)
    {
        position += glm::normalize(glm::cross(front, up)) * actualSpeed;
    }

    return position;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

// Transform------------------------------------------------------------------------------------------------------------------------------------------

glm::mat4 TransformComponent::GetLocalTransformMatrix() const
{
    const glm::mat4 rotationMatrix = glm::toMat4(glm::quat(rotation));

    return glm::translate(glm::mat4(1.0f), position) * rotationMatrix * glm::scale(glm::mat4(1.0f), scale);
}

const glm::mat4& TransformComponent::GetWorldTransformMatrix() const
{
    return transformMatrix;
}

void TransformComponent::RecalculateTransform(TransformComponent& parentTransform)
{
    const auto parentTransformMatrix = glm::translate(glm::mat4(1.0f), parentTransform.position);
    transformMatrix = parentTransformMatrix * GetLocalTransformMatrix();
}

void TransformComponent::RecalculateTransform()
{
    transformMatrix = GetLocalTransformMatrix();
}

//----------------------------------------------------------------------------------------------------------------------------------------------------