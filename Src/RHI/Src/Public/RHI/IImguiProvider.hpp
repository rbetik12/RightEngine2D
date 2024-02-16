#pragma once

#include <RHI/Config.hpp>
#include <RHI/RenderPass.hpp>
#include <Core/Type.hpp>

namespace rhi::imgui
{

class RHI_API IImguiProvider : public core::NonCopyable
{
public:
	virtual ~IImguiProvider() {}

	virtual void Begin() = 0;
	virtual void End() = 0;

	void SetRenderPass(const std::shared_ptr<rhi::RenderPass>& pass) { m_renderPass = pass; }

	static std::shared_ptr<IImguiProvider> Create();

protected:
	std::shared_ptr<rhi::RenderPass> m_renderPass;
};

} // rhi::imgui