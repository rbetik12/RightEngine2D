#pragma once

#include <RHI/Config.hpp>
#include <Rhi/Device.hpp>

namespace rhi::vulkan
{

class RHI_API VulkanDevice : public Device
{
public:
	virtual ~VulkanDevice() override;

	virtual std::shared_ptr<ShaderCompiler> CreateShaderCompiler(const ShaderCompiler::Options& options = {}) override;

private:

};

}
