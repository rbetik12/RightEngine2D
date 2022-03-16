#include "Camera.hpp"
#include "Input.hpp"
#include "EventDispatcher.hpp"
#include "KeyCodes.hpp"
#include <glm/gtc/matrix_transform.hpp>

RightEngine::FPSCamera::FPSCamera(const glm::vec3& position, const glm::vec3& worldUp) : position(position),
                                                                                         worldUp(worldUp),
                                                                                         rotation(90.0f, 0.0f, 0.0f)
{
    UpdateVectors();
    EventDispatcher::Get().Subscribe(MouseMovedEvent::descriptor, EVENT_CALLBACK(FPSCamera::OnEvent));
}

void RightEngine::FPSCamera::OnUpdate()
{
    actualSpeed = movementSpeed * Input::deltaTime;
    Move();
}

glm::mat4 RightEngine::FPSCamera::GetViewMatrix() const
{
    return glm::lookAt(position, position + front, up);
}

const glm::vec3& RightEngine::FPSCamera::GetPosition() const
{
    return position;
}

const glm::vec3& RightEngine::FPSCamera::GetRotation() const
{
    return rotation;
}

const glm::vec3& RightEngine::FPSCamera::GetFront() const
{
    return front;
}

const float& RightEngine::FPSCamera::GetCameraMovementSpeed() const
{
    return movementSpeed;
}

void RightEngine::FPSCamera::ToggleMouseCapture()
{
    shouldCaptureMouse = !shouldCaptureMouse;
}

bool RightEngine::FPSCamera::IsMouseCaptured()
{
    return shouldCaptureMouse;
}

void RightEngine::FPSCamera::UpdateVectors()
{
    float yaw = rotation[0];
    float pitch = rotation[1];
    glm::vec3 rotatedFront;
    rotatedFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    rotatedFront.y = sin(glm::radians(pitch));
    rotatedFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

    front = glm::normalize(rotatedFront);
    right = glm::normalize(glm::cross(front, worldUp));
    up = glm::normalize(glm::cross(right, front));
}

bool RightEngine::FPSCamera::OnEvent(const Event& event)
{
    if (event.GetType() == MouseMovedEvent::descriptor)
    {
        return OnMouseMove(static_cast<const MouseMovedEvent&>(event));
    }

    return true;
}

bool RightEngine::FPSCamera::OnMouseMove(const MouseMovedEvent& e)
{
    if (!shouldCaptureMouse)
    {
        UpdateVectors();
        return true;
    }
    if (prevXMousePos == -1)
    {
        prevXMousePos = e.GetX();
        prevYMousePos = e.GetY();
    }
    float xOffset = e.GetX() - prevXMousePos;
    float yOffset = prevYMousePos - e.GetY();
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
    UpdateVectors();
    prevXMousePos = e.GetX();
    prevYMousePos = e.GetY();
    return true;
}

void RightEngine::FPSCamera::Move()
{
    if(Input::IsKeyDown(R_KEY_W))
    {
        position += actualSpeed * front;
    }
    if(Input::IsKeyDown(R_KEY_S))
    {
        position -= actualSpeed * front;
    }
    if(Input::IsKeyDown(R_KEY_A))
    {
        position -= glm::normalize(glm::cross(front, up)) * actualSpeed;
    }
    if(Input::IsKeyDown(R_KEY_D))
    {
        position += glm::normalize(glm::cross(front, up)) * actualSpeed;
    }

}