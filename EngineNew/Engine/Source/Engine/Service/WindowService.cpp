#include <Engine/Service/WindowService.hpp>
#include <Engine/Engine.hpp>

namespace engine
{

WindowService::~WindowService()
{
}

void WindowService::Init()
{
	ENGINE_ASSERT(glfwInit());

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	m_window = glfwCreateWindow(1920, 1080, "Right Engine", nullptr, nullptr);

	ENGINE_ASSERT(m_window);

	glfwSetWindowCloseCallback(m_window, [](GLFWwindow* window)
	{
		Instance().Stop();
	});

	core::log::info("Successfully initialized WindowService");
}

void WindowService::Destroy()
{
	glfwDestroyWindow(m_window);

	core::log::info("Successfully destroyed WindowService");
}

void WindowService::Update(float dt)
{
	glfwPollEvents();
}

void WindowService::PostUpdate(float dt)
{
	glfwSwapBuffers(m_window);
}

} // namespace engine
