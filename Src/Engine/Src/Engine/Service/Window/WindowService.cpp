#include <Engine/Service/Window/WindowService.hpp>
#include <Engine/Service/Render/RenderService.hpp>
#include <Engine/Engine.hpp>
#include <Engine/Registration.hpp>
#include <imgui.h>

RTTR_REGISTRATION
{
engine::registration::Service<engine::WindowService>("engine::WindowService")
    .UpdateBefore<engine::RenderService>()
    .Domain(engine::Domain::UI);
}

namespace 
{

eastl::array<bool, 512>     keyArray;
eastl::array<bool, 8>       mouseArray;
glm::vec2                   mousePos;
glm::vec2                   prevMousePos;

} // unnamed

namespace engine
{

WindowService::WindowService()
{
    ENGINE_ASSERT(glfwInit());

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    m_window = glfwCreateWindow(1920, 1080, "Right Engine", nullptr, nullptr);

    ENGINE_ASSERT(m_window);

    std::memset(keyArray.data(), 0, keyArray.size());
    std::memset(mouseArray.data(), 0, mouseArray.size());

    glfwSetWindowCloseCallback(m_window, [](GLFWwindow* window)
        {
            Instance().Stop();
        });

    glfwSetWindowSizeCallback(m_window, [](GLFWwindow* window, int width, int height)
        {
            auto& rs = Instance().Service<RenderService>();
            rs.OnWindowResize({ width, height });
        });

    glfwSetKeyCallback(m_window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
        {
            switch (action)
            {
                case GLFW_PRESS:
                {
                    keyArray[key] = true;
                    break;
                }
                case GLFW_RELEASE:
                {
                    keyArray[key] = false;
                    break;
                }
                case GLFW_REPEAT:
                {
                    keyArray[key] = true;
                    break;
                }
                default:
                    ENGINE_ASSERT(false);
                    break;
            }
        });

    glfwSetMouseButtonCallback(m_window, [](GLFWwindow* window, int button, int action, int mods)
        {
            switch (action)
            {
                case GLFW_PRESS:
                {
                    mouseArray[button] = true;
                    break;
                }
                case GLFW_RELEASE:
                {
                    mouseArray[button] = false;
                    break;
                }
                case GLFW_REPEAT:
                {
                    mouseArray[button] = true;
                    break;
                }
                default:
                {
                    ENGINE_ASSERT(false);
                    break;
                }
            }
        });

    glfwSetCursorPosCallback(m_window, [](GLFWwindow* window, double xPos, double yPos)
        {
            prevMousePos = mousePos;
            mousePos.x = static_cast<float>(xPos);
            mousePos.y = static_cast<float>(yPos);
        });
}

WindowService::~WindowService()
{
    glfwDestroyWindow(m_window);
}

void WindowService::Update(float dt)
{
    PROFILER_CPU_ZONE;

    glfwPollEvents();
}

void WindowService::PostUpdate(float dt)
{
    PROFILER_CPU_ZONE;

    glfwSwapBuffers(m_window);
}

glm::ivec2 WindowService::Extent() const
{
    glm::ivec2 extent;
    glfwGetWindowSize(m_window, &extent.x, &extent.y);
    return extent;
}

glm::vec2 WindowService::WindowScale() const
{
    glm::vec2 scale;
    glfwGetWindowContentScale(m_window, &scale.x, &scale.y);
    return scale;
}

bool WindowService::KeyButtonPressed(KeyButton button) const
{
    return keyArray[static_cast<int>(button)];
}

bool WindowService::MouseButtonPressed(MouseButton button) const
{
    return mouseArray[static_cast<int>(button)];
}

glm::vec2 WindowService::MousePos() const
{
    return mousePos;
}

glm::vec2 WindowService::PrevMousePos() const
{
    return prevMousePos;
}

} // namespace engine
