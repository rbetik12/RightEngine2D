#pragma once

#include <Engine/Service/IService.hpp>
#include <GLFW/glfw3.h>

namespace engine
{

// Service is responsible for current window drawing and input events processing
class ENGINE_API WindowService final : public IService
{
public:
	virtual ~WindowService() override;

	virtual void Init() override;
	virtual void Destroy() override;

	virtual void Update(float dt) override;
	virtual void PostUpdate(float dt) override;

private:
	GLFWwindow* m_window = nullptr;
};

} // namespace engine
