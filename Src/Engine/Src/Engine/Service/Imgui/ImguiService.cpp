#include <Engine/Service/Imgui/ImguiService.hpp>
#include <Engine/Registration.hpp>
#include <Engine/Engine.hpp>
#include <Engine/Service/WindowService.hpp>
#include <Engine/Service/Render/RenderService.hpp>
#include <Core/RTTRIntegration.hpp>
#include <imgui.h>
#include "imgui_impl_glfw.h"

RTTR_REGISTRATION
{
engine::registration::Service<engine::ImguiService>("engine::ImguiService")
    .UpdateAfter<engine::RenderService>()
    .PostUpdateBefore<engine::RenderService>()
    .Domain(engine::Domain::EDITOR);
}

namespace
{

const ImVec4        C_UI_TEXT_COLOR = ImColor(255, 255, 255);
const ImVec4        C_UI_WINDOW_BG_COLOR = ImColor(20, 20, 20);
const ImVec4        C_UI_ELEMENT_BG_COLOR = ImColor(50, 50, 50);
const ImVec4        C_UI_ELEMENT_HOVER_COLOR = ImColor(100, 100, 100);
const ImVec4        C_UI_ELEMENT_ACTIVE_COLOR = ImColor(130, 130, 130);
constexpr uint8_t   C_DEFAULT_FONT_SIZE = 14;

} // unnamed

namespace engine
{

ImguiService::ImguiService()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking

    auto& ws = Instance().Service<WindowService>();
    const auto displayScale = ws.WindowScale();

    io.DisplaySize = { displayScale.x, displayScale.y };

    // TODO: Add font loading after VFS will be implemented
    /* io.Fonts->AddFontFromFileTTF("./Inter-Regular.ttf", C_DEFAULT_FONT_SIZE * displayScale.x); */

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
    style.ScaleAllSizes(displayScale.x);

    ImGui_ImplGlfw_InitForVulkan(ws.Window(), true);

    m_imguiProvider = rhi::imgui::IImguiProvider::Create();
}

ImguiService::~ImguiService()
{
    ImGui_ImplGlfw_Shutdown();
    m_imguiProvider.reset();
}

void ImguiService::Update(float dt)
{
    auto& rs = Instance().Service<RenderService>();

    ImGui_ImplGlfw_NewFrame();
    m_imguiProvider->SetRenderPass(rs.ImGuiPass());
    m_imguiProvider->Begin();
    ImGui::NewFrame();
}

void ImguiService::PostUpdate(float dt)
{
    ImGui::Render();
    m_imguiProvider->End();
}

} // engine