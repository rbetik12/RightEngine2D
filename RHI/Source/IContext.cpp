#pragma once

#include <RHI/IContext.hpp>
#include "Vulkan/VulkanContext.hpp"

namespace rhi
{

namespace vulkan
{

std::shared_ptr<IContext> CreateContext(VulkanInitContext&& ctx)
{
	return std::make_shared<rhi::vulkan::VulkanContext>(std::move(ctx));
}

} // namespace vulkan

} // namespace rhi