#pragma once

#include <Engine/Config.hpp>
#include <Engine/Service/IService.hpp>
#include <RHI/IImguiProvider.hpp>

namespace engine
{

class ENGINE_API ImguiService : public IService
{
	RTTR_ENABLE(IService);
public:
	ImguiService();
	virtual ~ImguiService() override;

	virtual void Update(float dt) override;
	virtual void PostUpdate(float dt) override;

private:
	std::shared_ptr<rhi::imgui::IImguiProvider> m_imguiProvider;
};

} // engine