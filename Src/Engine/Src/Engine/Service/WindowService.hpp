#pragma once

#include <Engine/Service/IService.hpp>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

namespace engine
{

// Service is responsible for current window drawing and input events processing
class ENGINE_API WindowService final : public IService
{
	RTTR_ENABLE(IService);
public:
	WindowService();
	virtual ~WindowService() override;

	virtual void		Update(float dt) override;
	virtual void		PostUpdate(float dt) override;

	glm::ivec2          Extent() const;
	glm::vec2			WindowScale() const;

	GLFWwindow*			Window() { return m_window; }
	const GLFWwindow*	Window() const { return m_window; }

private:
	GLFWwindow* m_window = nullptr;
};

} // namespace engine
