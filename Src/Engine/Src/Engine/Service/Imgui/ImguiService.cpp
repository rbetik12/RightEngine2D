#include <Engine/Service/Imgui/ImguiService.hpp>
#include <Engine/Registration.hpp>
#include <Engine/Engine.hpp>
#include <Engine/Service/Window/WindowService.hpp>
#include <Engine/Service/Render/RenderService.hpp>
#include <Engine/Service/Filesystem/VirtualFilesystemService.hpp>
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

enum class CommonColor : uint8_t
{
	Transparent = 0,

	Red400,
	Red200,
	Red50,

	Orange400,
	Orange200,
	Orange50,

	Yellow400,
	Yellow200,
	Yellow50,

	Green400,
	Green200,
	Green50,

	Aquamarine400,
	Aquamarine200,
	Aquamarine50,

	Cyan400,
	Cyan200,
	Cyan50,

	Blue400,
	Blue200,
	Blue50,

	Magenta400,
	Magenta200,
	Magenta50,

	Pink400,
	Pink200,
	Pink50,

	Neutral0,
	Neutral100,
	Neutral200,
	Neutral300,
	Neutral400,
	Neutral500,
	Neutral600,
	Neutral700,
	Neutral800,
	Neutral1000,

	Count
};

constexpr ImVec4 Transparent() { return ImVec4(0.0f, 0.0f, 0.0f, 0.0f); }
constexpr ImVec4 Black() { return ImVec4(0.0f, 0.0f, 0.0f, 1.0f); }
constexpr ImVec4 White() { return ImVec4(1.0f, 1.0f, 1.0f, 1.0f); }
constexpr ImVec4 Red() { return ImVec4(1.0f, 0.0f, 0.0f, 1.0f); }
constexpr ImVec4 Green() { return ImVec4(0.0f, 1.0f, 0.0f, 1.0f); }
constexpr ImVec4 Blue() { return ImVec4(0.0f, 0.0f, 1.0f, 1.0f); }
constexpr ImVec4 Yellow() { return ImVec4(1.0f, 1.0f, 0.0f, 1.0f); }
constexpr ImVec4 Cyan() { return ImVec4(0.0f, 1.0f, 1.0f, 1.0f); }
constexpr ImVec4 Magenta() { return ImVec4(1.0f, 0.0f, 1.0f, 1.0f); }

constexpr ImVec4 FromRGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	return ImVec4(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
}

constexpr ImVec4 FromRGBA(uint32_t rgba)
{
	return FromRGBA(
		(rgba >> 24) & 0xFF,
		(rgba >> 16) & 0xFF,
		(rgba >> 8) & 0xFF,
		rgba & 0xFF);
}

constexpr ImVec4 DisabledColor(const ImVec4& color, bool disable = true)
{
	return disable ? ImVec4(color.x, color.y, color.z, 0.5f) : color;
}

constexpr ImVec4 FromCommonColor(CommonColor value, bool disable = false)
{
	using Colors = eastl::array<ImVec4, static_cast<size_t>(CommonColor::Count)>;
	constexpr Colors C_COLORS =
	{
		// NamingConvention: Dark - 400, Normal - 200, Light - 50
		Transparent(),			// Transparent,
		FromRGBA(0xB6221CFF),	// Red400,
		FromRGBA(0xF7625CFF),	// Red200,
		FromRGBA(0xFAA19CFF),	// Red50,
		FromRGBA(0xBA671AFF),	// Orange400,
		FromRGBA(0xF7A356FF),	// Orange200,
		FromRGBA(0xFBC798FF),	// Orange50,
		FromRGBA(0xC9991CFF),	// Yellow400,
		FromRGBA(0xF8CD5EFF),	// Yellow200,
		FromRGBA(0xFBE19BFF),	// Yellow50,
		FromRGBA(0x238F29FF),	// Green400,
		FromRGBA(0x65CC6BFF),	// Green200,
		FromRGBA(0xA1E0A6FF),	// Green50,
		FromRGBA(0x14B46FFF),	// Aquamarine400,
		FromRGBA(0x66DDAAFF),	// Aquamarine200,
		FromRGBA(0x73F4C9FF),	// Aquamarine50,
		FromRGBA(0x28989FFF),	// Cyan400,
		FromRGBA(0x5DCFD7FF),	// Cyan200,
		FromRGBA(0xA0E2E7FF),	// Cyan50,
		FromRGBA(0x185FBCFF),	// Blue400,
		FromRGBA(0x4691F4FF),	// Blue200,
		FromRGBA(0x91BDF8FF),	// Blue50,
		FromRGBA(0x76289FFF),	// Magenta400,
		FromRGBA(0xB175D1FF),	// Magenta200,
		FromRGBA(0xD1ACE3FF),	// Magenta50,
		FromRGBA(0xB11A6CFF),	// Pink400,
		FromRGBA(0xEA1D8CFF),	// Pink200,
		FromRGBA(0xE088B8FF),	// Pink50,
		Black(),				// Neutral0,
		FromRGBA(0x1A1A1AFF),	// Neutral100
		FromRGBA(0x272727FF),	// Neutral200
		FromRGBA(0x313131FF),	// Neutral300
		FromRGBA(0x414141FF),	// Neutral400
		FromRGBA(0x484848FF),	// Neutral500
		FromRGBA(0x808080FF),	// Neutral600
		FromRGBA(0xB3B3B3FF),	// Neutral700
		FromRGBA(0xC0C0C0FF),	// Neutral800
		White()					//Neutral1000
	};

	return DisabledColor(C_COLORS[static_cast<size_t>(value)], disable);
}

constexpr ImVec4 MakeImVec4(const ImVec4& vec, float w)
{
	return ImVec4(vec.x, vec.y, vec.x, w);
}

constexpr uint8_t C_DEFAULT_FONT_SIZE = 14;

} // unnamed

namespace engine
{

ImguiService::ImguiService()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    auto& vfs = Instance().Service<io::VirtualFilesystemService>();
    m_configFilePath = vfs.Absolute("/Config/imgui.ini").generic_u8string();

    io.IniFilename = m_configFilePath.c_str();
    ImGui::LoadIniSettingsFromDisk(io.IniFilename);

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking

    auto& ws = Instance().Service<WindowService>();
    const auto displayScale = ws.WindowScale();

    io.DisplaySize = { displayScale.x, displayScale.y };

    // TODO: Move that to project config file later
    io.Fonts->AddFontFromFileTTF(vfs.Absolute("/System/Fonts/Inter-Regular.ttf").generic_u8string().c_str(), C_DEFAULT_FONT_SIZE * displayScale.x);

    ImGuiStyle& style = ImGui::GetStyle();
	style.Colors[ImGuiCol_NavHighlight] = FromCommonColor(CommonColor::Orange200);
	style.Colors[ImGuiCol_Text] = FromCommonColor(CommonColor::Neutral800);
	style.Colors[ImGuiCol_TextDisabled] = FromCommonColor(CommonColor::Neutral600);
	style.Colors[ImGuiCol_WindowBg] = FromCommonColor(CommonColor::Neutral300);
	style.Colors[ImGuiCol_ChildBg] = style.Colors[ImGuiCol_WindowBg];
	style.Colors[ImGuiCol_PopupBg] = style.Colors[ImGuiCol_WindowBg];
	style.Colors[ImGuiCol_Border] = FromCommonColor(CommonColor::Neutral100);
	style.Colors[ImGuiCol_BorderShadow] = FromRGBA(0x00000040);
	style.Colors[ImGuiCol_FrameBg] = FromCommonColor(CommonColor::Neutral200);
	style.Colors[ImGuiCol_FrameBgHovered] = FromCommonColor(CommonColor::Neutral100);
	style.Colors[ImGuiCol_FrameBgActive] = FromCommonColor(CommonColor::Neutral100);
	style.Colors[ImGuiCol_TitleBg] = FromCommonColor(CommonColor::Neutral100);
	style.Colors[ImGuiCol_TitleBgActive] = FromCommonColor(CommonColor::Neutral100);
	style.Colors[ImGuiCol_TitleBgCollapsed] = FromCommonColor(CommonColor::Neutral100);
	style.Colors[ImGuiCol_MenuBarBg] = style.Colors[ImGuiCol_WindowBg];
	style.Colors[ImGuiCol_Header] = FromCommonColor(CommonColor::Neutral500);
	style.Colors[ImGuiCol_HeaderHovered] = FromCommonColor(CommonColor::Neutral500);
	style.Colors[ImGuiCol_HeaderActive] = FromCommonColor(CommonColor::Neutral500);
	style.Colors[ImGuiCol_Button] = FromCommonColor(CommonColor::Neutral500);
	style.Colors[ImGuiCol_ButtonHovered] = FromCommonColor(CommonColor::Neutral500);
	style.Colors[ImGuiCol_ButtonActive] = FromCommonColor(CommonColor::Neutral500);
	style.Colors[ImGuiCol_ScrollbarBg] = style.Colors[ImGuiCol_WindowBg];
	style.Colors[ImGuiCol_ScrollbarGrab] = style.Colors[ImGuiCol_Button];
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = style.Colors[ImGuiCol_ButtonHovered];
	style.Colors[ImGuiCol_ScrollbarGrabActive] = style.Colors[ImGuiCol_ButtonHovered];
	style.Colors[ImGuiCol_Separator] = FromCommonColor(CommonColor::Neutral500);
	style.Colors[ImGuiCol_SeparatorHovered] = FromCommonColor(CommonColor::Neutral600);
	style.Colors[ImGuiCol_SeparatorActive] = style.Colors[ImGuiCol_SeparatorHovered];
	style.Colors[ImGuiCol_CheckMark] = FromCommonColor(CommonColor::Neutral600);
	style.Colors[ImGuiCol_SliderGrab] = style.Colors[ImGuiCol_Button];
	style.Colors[ImGuiCol_SliderGrabActive] = style.Colors[ImGuiCol_ButtonHovered];
	style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
	style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
	style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
	style.Colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
	style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.73f, 0.73f, 0.73f, 0.35f);
	style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.6f);
	style.Colors[ImGuiCol_DragDropTarget] = style.Colors[ImGuiCol_NavHighlight];
	style.Colors[ImGuiCol_NavWindowingHighlight] = style.Colors[ImGuiCol_WindowBg];
	style.Colors[ImGuiCol_NavWindowingDimBg] = Transparent();
	style.Colors[ImGuiCol_DockingEmptyBg] = style.Colors[ImGuiCol_TitleBg];
	style.Colors[ImGuiCol_Tab] = style.Colors[ImGuiCol_TitleBg];
	style.Colors[ImGuiCol_TabHovered] = style.Colors[ImGuiCol_WindowBg];
	style.Colors[ImGuiCol_TabActive] = style.Colors[ImGuiCol_WindowBg];
	style.Colors[ImGuiCol_TabUnfocused] = style.Colors[ImGuiCol_TitleBg];
	style.Colors[ImGuiCol_TabUnfocusedActive] = style.Colors[ImGuiCol_WindowBg];
	style.Colors[ImGuiCol_DockingPreview] = MakeImVec4(style.Colors[ImGuiCol_NavHighlight], 0.8f);
	style.Colors[ImGuiCol_TableHeaderBg] = FromCommonColor(CommonColor::Neutral200);
	style.Colors[ImGuiCol_TableRowBg] = FromRGBA(0x383838FF);
	style.Colors[ImGuiCol_TableRowBgAlt] = FromRGBA(0x404040FF);
	style.Colors[ImGuiCol_TableBorderStrong] = style.Colors[ImGuiCol_Separator];
	style.Colors[ImGuiCol_TableBorderLight] = style.Colors[ImGuiCol_Separator];
    style.FrameRounding = 0.0f;
    style.FrameBorderSize = 0.5f;
    style.ScaleAllSizes(displayScale.x);

    ImGui_ImplGlfw_InitForVulkan(ws.Window(), true);

    m_imguiProvider = rhi::imgui::IImguiProvider::Create();
}

ImguiService::~ImguiService()
{
    ImGui::SaveIniSettingsToDisk(m_configFilePath.c_str());

    ImGui_ImplGlfw_Shutdown();
    m_imguiProvider.reset();
}

void ImguiService::Update(float dt)
{
    PROFILER_CPU_ZONE;

	ImGui_ImplGlfw_NewFrame();

	auto& rs = Instance().Service<RenderService>();

	rs.RunOnRenderThreadWait([=, &rs]
		{
			m_imguiProvider->SetRenderPass(rs.ImGuiPass());
			m_imguiProvider->Begin();
		});

	ImGui::NewFrame();
}

void ImguiService::PostUpdate(float dt)
{
    PROFILER_CPU_ZONE;

	auto& rs = Instance().Service<RenderService>();

	ImGui::Render();

	rs.RunOnRenderThreadWait([=]
		{
			m_imguiProvider->End();
		});
}

void ImguiService::Image(const std::shared_ptr<rhi::Texture>& texture, const ImVec2& size, const ImVec2& uv0,
    const ImVec2& uv1)
{
	m_imguiProvider->Image(texture, size, uv0, uv1);
}

void ImguiService::RemoveImage(const std::shared_ptr<rhi::Texture>& texture)
{
    ENGINE_ASSERT(texture);
	m_imguiProvider->RemoveImage(texture);
}

} // engine