#pragma once

#include <RHI/Config.hpp>
#include <RHI/Shader.hpp>

namespace rhi::vulkan
{

	class RHI_API VulkanShader : public Shader
	{
	public:
		VulkanShader(const std::shared_ptr<Device>& device, const ShaderDescriptor& descriptor);

		virtual ~VulkanShader() override;

	private:
	};

}