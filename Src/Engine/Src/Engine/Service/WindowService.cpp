#include <Engine/Service/WindowService.hpp>
#include <Engine/Engine.hpp>
#include <Engine/Registration.hpp>

#include "Render/RenderService.hpp"

RTTR_REGISTRATION
{
engine::registration::Service<engine::WindowService>("engine::WindowService")
	.UpdateBefore<engine::RenderService>()
	.Domain(engine::Domain::UI);
}

namespace engine
{

WindowService::WindowService()
{
	ENGINE_ASSERT(glfwInit());

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	m_window = glfwCreateWindow(1920, 1080, "Right Engine", nullptr, nullptr);

	ENGINE_ASSERT(m_window);

	glfwSetWindowCloseCallback(m_window, [](GLFWwindow* window)
		{
			Instance().Stop();
		});

	glfwSetWindowSizeCallback(m_window, [](GLFWwindow* window, int width, int height)
		{
			auto& rs = Instance().Service<RenderService>();
			rs.OnResize(width, height);
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
} // namespace engine
