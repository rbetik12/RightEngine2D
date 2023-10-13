#pragma once

#include <RHI/Config.hpp>
#include <RHI/RenderPassDescriptor.hpp>

namespace rhi
{

class RHI_API RenderPass
{
public:
	virtual ~RenderPass() = default;

protected:
    RenderPassDescriptor m_descriptor;

    RenderPass(const RenderPassDescriptor& descriptor) : m_descriptor(descriptor)
	{}
};

}