#pragma once

#include <Engine/Service/IService.hpp>
#include <RHI/ShaderCompiler.hpp>

namespace engine
{

class ENGINE_API EditorService final : public IService
{
	RTTR_ENABLE(IService);
public:
	struct Impl;

	EditorService();
	virtual ~EditorService() override;

	virtual void	Update(float dt) override;
	virtual void	PostUpdate(float dt) override;

	void Initialize();

private:
	std::unique_ptr<Impl> m_impl;
};

} // namespace engine