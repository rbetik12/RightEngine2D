#pragma once

#include <RHI/Config.hpp>
#include <RHI/ShaderDescriptor.hpp>
#include <RHI/Device.hpp>

namespace rhi
{

	class RHI_API Shader
	{
	public:
		Shader(const std::shared_ptr<Device>& device, const ShaderDescriptor& descriptor) : m_descriptor(descriptor)
		{}

		virtual ~Shader() = default;

		inline const ShaderDescriptor& GetShaderProgramDescriptor() const { return m_descriptor; }

	private:
		ShaderDescriptor m_descriptor;
	};

}