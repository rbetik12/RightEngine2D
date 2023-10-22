#pragma once

#include <RHI/Config.hpp>
#include <RHI/PipelineDescriptor.hpp>
#include <cstdint>

namespace rhi
{

class RHI_API Pipeline
{
public:
	virtual ~Pipeline() = default;

protected:
	PipelineDescriptor m_descriptor;

	Pipeline(const PipelineDescriptor& descriptor) : m_descriptor(descriptor)
	{
		RHI_ASSERT(descriptor.m_shader && descriptor.m_pass);
	}
};

}