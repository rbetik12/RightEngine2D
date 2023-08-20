#include "VulkanImguiLayerImpl.hpp"
#include "Device.hpp"
#include "Application.hpp"
#include "RendererCommand.hpp"
#include "VulkanDevice.hpp"
#include "VulkanCommandBuffer.hpp"
#include "VulkanUtils.hpp"
#include "VulkanGraphicsPipeline.hpp"
#include "VulkanTexture.hpp"
#include "VulkanSampler.hpp"
#include "Path.hpp"
#include "ImGuiLayer.hpp"
#include <imgui.h>
#include <ImGuiBackend/imgui_impl_vulkan.h>
#include <ImGuiBackend/imgui_impl_glfw.h>
#include <ImGuizmo.h>
#include <vulkan/vulkan.h>

using namespace RightEngine;

const ImVec4 C_UI_TEXT_COLOR = ImColor(255, 255, 255);
const ImVec4 C_UI_WINDOW_BG_COLOR = ImColor(20, 20, 20);
const ImVec4 C_UI_ELEMENT_BG_COLOR = ImColor(50, 50, 50);
const ImVec4 C_UI_ELEMENT_HOVER_COLOR = ImColor(100, 100, 100);
const ImVec4 C_UI_ELEMENT_ACTIVE_COLOR = ImColor(130, 130, 130);

void VulkanImguiLayerImpl::OnAttach(const std::shared_ptr<GraphicsPipeline>& pipeline)
{
    VkDescriptorPoolSize pool_sizes[] =
            {
                    { VK_DESCRIPTOR_TYPE_SAMPLER, 100 },
                    { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1024 },
                    { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 100 },
                    { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 100 },
                    { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 100 },
                    { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 100 },
                    { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 100 },
                    { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 100 },
                    { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 100 },
                    { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 100 },
                    { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 100 }
            };

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000;
    pool_info.poolSizeCount = std::size(pool_sizes);
    pool_info.pPoolSizes = pool_sizes;

    const auto vkDevice = VK_DEVICE();

    R_CORE_ASSERT(vkCreateDescriptorPool(vkDevice->GetDevice(), &pool_info, nullptr, &imguiPool) == VK_SUCCESS, "");

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    
    io.Fonts->AddFontFromFileTTF(Path::Absolute("/Fonts/Roboto-Regular.ttf").c_str(), 20);
    iniFilePath = Path::Absolute("/Config/imgui.ini");
    io.IniFilename = iniFilePath.c_str();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    //io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
    //io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoTaskBarIcons;
    //io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoMerge;

    ImGuiStyle& style = ImGui::GetStyle();
    style.Colors[ImGuiCol_Text] = C_UI_TEXT_COLOR;
    style.Colors[ImGuiCol_WindowBg] = C_UI_WINDOW_BG_COLOR;
    style.Colors[ImGuiCol_ChildBg] = C_UI_WINDOW_BG_COLOR;
    style.Colors[ImGuiCol_PopupBg] = C_UI_WINDOW_BG_COLOR;
    style.Colors[ImGuiCol_Button] = C_UI_ELEMENT_BG_COLOR;
    style.Colors[ImGuiCol_ButtonHovered] = C_UI_ELEMENT_HOVER_COLOR;
    style.Colors[ImGuiCol_ButtonActive] = C_UI_ELEMENT_ACTIVE_COLOR;
    style.Colors[ImGuiCol_FrameBg] = C_UI_ELEMENT_BG_COLOR;
    style.Colors[ImGuiCol_FrameBgHovered] = C_UI_ELEMENT_HOVER_COLOR;
    style.Colors[ImGuiCol_FrameBgActive] = C_UI_ELEMENT_ACTIVE_COLOR;
    style.Colors[ImGuiCol_Header] = C_UI_ELEMENT_BG_COLOR;
    style.Colors[ImGuiCol_HeaderHovered] = C_UI_ELEMENT_HOVER_COLOR;
    style.Colors[ImGuiCol_HeaderActive] = C_UI_ELEMENT_ACTIVE_COLOR;
    style.FrameRounding = 0.f;
    style.FrameBorderSize = 0.5f;

    ImGui::LoadIniSettingsFromDisk(io.IniFilename);

    Application& app = Application::Get();
    GLFWwindow* window = static_cast<GLFWwindow*>(app.GetWindow()->GetNativeHandle());

    ImGui_ImplGlfw_InitForVulkan(window, true);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = std::static_pointer_cast<VulkanRenderingContext>(RendererCommand::GetContext())->GetInstance();
    init_info.PhysicalDevice = vkDevice->GetPhysicalDevice();
    init_info.Device = vkDevice->GetDevice();
    init_info.Queue = vkDevice->GetQueue(QueueType::GRAPHICS);
    init_info.DescriptorPool = imguiPool;
    init_info.MinImageCount = 3;
    init_info.ImageCount = 3;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    ImGui_ImplVulkan_Init(&init_info, std::static_pointer_cast<VulkanGraphicsPipeline>(pipeline)->GetRenderPass());

    const auto cmd = std::static_pointer_cast<VulkanCommandBuffer>(vkDevice->CreateCommandBuffer({ CommandBufferType::GRAPHICS }));
    VulkanUtils::BeginCommandBuffer(cmd, true);
    cmd->Enqueue([](auto buffer)
    {
        ImGui_ImplVulkan_CreateFontsTexture(VK_CMD(buffer)->GetBuffer());
    });
    VulkanUtils::EndCommandBuffer(vkDevice, cmd);

    ImGui_ImplVulkan_DestroyFontUploadObjects();
}

void VulkanImguiLayerImpl::OnDetach()
{
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SaveIniSettingsToDisk(io.IniFilename);
    vkDestroyRenderPass(VK_DEVICE()->GetDevice(), renderPass, nullptr);
    vkDestroyDescriptorPool(VK_DEVICE()->GetDevice(), imguiPool, nullptr);
    ImGui_ImplVulkan_Shutdown();
}

void VulkanImguiLayerImpl::OnUpdate(float deltaTime)
{

}

void VulkanImguiLayerImpl::OnEvent(Event& event)
{

}

void VulkanImguiLayerImpl::Begin()
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGuizmo::BeginFrame();
}

void VulkanImguiLayerImpl::End(const std::shared_ptr<CommandBuffer>& cmd)
{
    ImGui::Render();
    cmd->Enqueue([](auto buffer)
    {
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(),
                                        VK_CMD(buffer)->GetBuffer());
    });
}

void VulkanImguiLayerImpl::Image(const std::shared_ptr<Texture>& texture, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1)
{
    const auto descSet = GetDescriptorSet(texture);
    ImGui::Image(descSet, size, uv0, uv1);
}

void VulkanImguiLayerImpl::ImageButton(const std::shared_ptr<Texture>& texture,
                                       const ImVec2& size,
                                       const ImVec2& uv0,
                                       const ImVec2& uv1)
{
    const auto descSet = GetDescriptorSet(texture);
    ImGui::ImageButton(descSet, size, uv0, uv1);
}

VkDescriptorSet VulkanImguiLayerImpl::GetDescriptorSet(const std::shared_ptr<Texture>& texture)
{
    R_CORE_ASSERT(texture->GetSampler() && texture->GetWidth() > 0 && texture->GetHeight() > 0, "")
	const auto vkTexture = std::static_pointer_cast<VulkanTexture>(texture);
    const auto vkSampler = std::static_pointer_cast<VulkanSampler>(vkTexture->GetSampler());
    if (imageViewToDescriptorSet.find(vkTexture->GetImageView()) == imageViewToDescriptorSet.end())
    {
        imageViewToDescriptorSet[vkTexture->GetImageView()] = ImGui_ImplVulkan_AddTexture(vkSampler->GetSampler(),
            vkTexture->GetImageView(),
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }
    return imageViewToDescriptorSet.at(vkTexture->GetImageView());
}

void ImGuiLayer::CreateImpl()
{
    if (!impl)
    {
        impl = std::make_shared<VulkanImguiLayerImpl>();
    }
}

std::shared_ptr<ImguiLayerImpl> ImGuiLayer::impl;


