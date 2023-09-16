#pragma once

#include <RHI/Config.hpp>
#include <RHI/ShaderCompiler.hpp>
#include <RHI/IContext.hpp>
#include <Core/Type.hpp>

namespace rhi
{
	class Buffer;

	struct DeviceProperties
	{
		size_t m_minUniformBufferOffsetAlignment = 0;
	};

	class RHI_API Device : public core::NonCopyable
	{
	public:
		virtual ~Device() = default;

		virtual std::shared_ptr<ShaderCompiler> CreateShaderCompiler(const ShaderCompiler::Options& options = {}) = 0;
		virtual std::shared_ptr<Buffer>			CreateBuffer(const BufferDescriptor& desc, const void* data) = 0;

		static std::shared_ptr<Device> Create(const std::shared_ptr<IContext>& ctx);

	protected:
		DeviceProperties m_properties;
	};
}
