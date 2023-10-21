#pragma once

#include <Engine/Service/IService.hpp>
#include <RHI/ShaderCompiler.hpp>

namespace engine
{

class ENGINE_API EditorService final : public IService
{
	RTTR_ENABLE(IService);
public:
	EditorService();
	virtual ~EditorService() override;

	virtual void	Update(float dt) override;
	virtual void	PostUpdate(float dt) override;

private:
	std::shared_ptr<rhi::ShaderCompiler> m_shaderCompiler;
};

} // namespace engine