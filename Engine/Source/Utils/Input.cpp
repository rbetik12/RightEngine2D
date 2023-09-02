#include "Input.hpp"
#include "EventDispatcher.hpp"
#include "KeyEvent.hpp"
#include "MouseEvent.hpp"
#include <GLFW/glfw3.h>
#include <cstring>

using namespace RightEngine;

glm::vec2 Input::currentMouseOffset = glm::vec2(0, 0);
glm::vec2 Input::mouseDeltaOffset = glm::vec2(0, 0);
float RightEngine::Input::lastFrame = 0.0f;
float RightEngine::Input::deltaTime = 0.0f;
float RightEngine::Input::frameTime = 0.0f;
bool RightEngine::Input::keyArray[512];
bool RightEngine::Input::mouseArray[8];

bool RightEngine::Input::IsKeyDown(int keyCode)
{
    return keyArray[keyCode];
}

bool RightEngine::Input::IsKeyUp(int keyCode)
{
    return !keyArray[keyCode];
}

bool RightEngine::Input::IsMouseButtonDown(RightEngine::MouseButton button)
{
    return mouseArray[static_cast<int>(button)];
}

bool RightEngine::Input::IsMouseButtonUp(RightEngine::MouseButton button)
{
    return !mouseArray[static_cast<int>(button)];
}

const glm::vec2& Input::GetMouseDelta()
{
    return mouseDeltaOffset;
}

RightEngine::Input& RightEngine::Input::Get()
{
    static Input input;
    return input;
}

void RightEngine::Input::Init()
{
    Input::Get();
}

RightEngine::Input::Input()
{
    std::memset(keyArray, 0, sizeof(keyArray));
    std::memset(mouseArray, 0, sizeof(mouseArray));
//    lastMouseOffset = glm::vec2(0.0f, 0.0f);
    EventDispatcher::Get().Subscribe(KeyPressedEvent::descriptor, EVENT_CALLBACK(Input::OnEvent));
    EventDispatcher::Get().Subscribe(KeyReleasedEvent::descriptor, EVENT_CALLBACK(Input::OnEvent));
    EventDispatcher::Get().Subscribe(MouseButtonPressedEvent::descriptor, EVENT_CALLBACK(Input::OnEvent));
    EventDispatcher::Get().Subscribe(MouseButtonReleasedEvent::descriptor, EVENT_CALLBACK(Input::OnEvent));
    EventDispatcher::Get().Subscribe(MouseScrollEvent::descriptor, EVENT_CALLBACK(Input::OnEvent));
}

float RightEngine::Input::GetTime()
{
    return glfwGetTime();
}

void RightEngine::Input::OnUpdate()
{
    float currentFrame = GetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
    frameTime = deltaTime * 1000;

    mouseDeltaOffset = { 0, 0 };
    mouseDeltaOffset = currentMouseOffset;
    currentMouseOffset = { 0, 0 };
}

bool RightEngine::Input::OnEvent(const Event& inputEvent)
{
    if (inputEvent.GetType() == KeyPressedEvent::descriptor)
    {
        const auto event = static_cast<const KeyPressedEvent&>(inputEvent);
        keyArray[event.GetKeyCode()] = true;
    }
    if (inputEvent.GetType() == KeyReleasedEvent::descriptor)
    {
        const auto event = static_cast<const KeyReleasedEvent&>(inputEvent);
        keyArray[event.GetKeyCode()] = false;
    }
    if (inputEvent.GetType() == MouseButtonPressedEvent::descriptor)
    {
        const auto event = static_cast<const MouseButtonPressedEvent&>(inputEvent);
        mouseArray[event.GetButton()] = true;
    }
    if (inputEvent.GetType() == MouseButtonReleasedEvent::descriptor)
    {
        const auto event = static_cast<const MouseButtonReleasedEvent&>(inputEvent);
        mouseArray[event.GetButton()] = false;
    }
    if (inputEvent.GetType() == MouseScrollEvent::descriptor)
    {
        const auto event = static_cast<const MouseScrollEvent&>(inputEvent);
        currentMouseOffset = event.GetOffset();
    }

    return true;
}
