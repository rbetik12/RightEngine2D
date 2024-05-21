#include <Engine/Editor/EntityTreePanel.hpp>
#include <Engine/Service/Imgui/ImguiService.hpp>
#include <Engine/Service/Render/RenderService.hpp>
#include <Engine/Service/EditorService.hpp>
#include <Engine/Service/WorldService.hpp>
#include <RHI/Pipeline.hpp>

namespace engine::editor
{

void EntityTreePanel::DrawPanel()
{
	auto& ws = Instance().Service<WorldService>();
    auto& editorService = Instance().Service<EditorService>();
    auto& world = ws.CurrentWorld();

    for (const auto entity : world->View())
    {
        if (ImGui::TreeNode(world->GetEntityManager()->GetEntityInfo(entity).m_name.c_str())) 
        {
            if (ImGui::IsItemClicked()) 
            {
                editorService.SelectedEntity(entity);
            }
            ImGui::TreePop();
        }
	}
}

} // engine::editor
