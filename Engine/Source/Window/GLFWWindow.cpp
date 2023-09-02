#include "GLFWWindow.hpp"
#include "Core.hpp"
#include "MouseEvent.hpp"
#include "KeyEvent.hpp"
#include "KeyCodes.hpp"

namespace RightEngine
{
    GLFWWindow::GLFWWindow(const std::string& title, uint32_t width, uint32_t height) : Window(title, width, height)
    {
        Init(title, width, height);
    }

    void GLFWWindow::Init(const std::string& title, uint32_t width, uint32_t height)
    {
        R_CORE_ASSERT(glfwInit(), "GLFW init failed");

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        window = glfwCreateWindow(1920, 1080, title.c_str(), nullptr, nullptr);
        R_CORE_ASSERT(window, "");

        glfwSetWindowCloseCallback(window, [](GLFWwindow* window)
        {
            EventDispatcher::Get().Emit(ShutdownEvent());
        });

        glfwSetCursorPosCallback(window, [](GLFWwindow* window, double xPos, double yPos)
        {
            EventDispatcher::Get().Emit(MouseMovedEvent(xPos, yPos));
        });

        glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
        {
            if (key == R_KEY_UNKNOWN)
            {
                switch (action)
                {
                    case GLFW_PRESS:
                    {
                        R_CORE_WARN("Unknown key press event!");
                        break;
                    }
                    case GLFW_RELEASE:
                    {
                        R_CORE_WARN("Unknown key release event!");
                        break;
                    }
                    case GLFW_REPEAT:
                    {
                        R_CORE_WARN("Unknown key repeat event!");
                        break;
                    }
                }
                return;
            }

            switch (action)
            {
                case GLFW_PRESS:
                {
                    EventDispatcher::Get().Emit(KeyPressedEvent(key, 0));
                    break;
                }
                case GLFW_RELEASE:
                {
                    EventDispatcher::Get().Emit(KeyReleasedEvent(key));
                    break;
                }
                case GLFW_REPEAT:
                {
                    EventDispatcher::Get().Emit(KeyPressedEvent(key, 1));
                    break;
                }
            }
        });

        glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods)
        {
            switch (action)
            {
                case GLFW_PRESS:
                {
                    EventDispatcher::Get().Emit(MouseButtonPressedEvent(button));
                    break;
                }
                case GLFW_RELEASE:
                {
                    EventDispatcher::Get().Emit(MouseButtonReleasedEvent(button));
                    break;
                }
            }
        });

        glfwSetScrollCallback(window, [](GLFWwindow* window, double xOffset, double yOffset)
        {
            EventDispatcher::Get().Emit(MouseScrollEvent(xOffset, yOffset));
        });
    }

    void GLFWWindow::OnUpdate()
    {
        glfwPollEvents();
    }

    void GLFWWindow::Swap() const
    {
        glfwSwapBuffers(window);
    }

    GLFWWindow::~GLFWWindow()
    {
        glfwDestroyWindow(window);
    }

    void* GLFWWindow::GetNativeHandle() const
    {
        return window;
    }
}
