#include <RHI/IImguiProvider.hpp>
#include <Vulkan/Imgui/VulkanImguiProvider.hpp>

namespace rhi::imgui
{

std::shared_ptr<IImguiProvider> IImguiProvider::Create()
{
    static bool created = false;
    RHI_ASSERT(!created);
    created = true;

    return std::make_shared<vulkan::imgui::VulkanImguiProvider>();
}

} // rhi::imgui