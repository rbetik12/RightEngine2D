#pragma once

#include <RHI/Config.hpp>
#include <RHI/ShaderDescriptor.hpp>
#include <RHI/Device.hpp>

namespace rhi
{

	class RHI_API Shader
	{
	public:
		virtual ~Shader() = default;

		inline const ShaderDescriptor& GetShaderDescriptor() const { return m_descriptor; }

	protected:
		ShaderDescriptor m_descriptor;

		Shader(const ShaderDescriptor& descriptor) : m_descriptor(descriptor)
		{}
	};

}
