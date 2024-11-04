#include <Engine/Editor/ComponentPanel.hpp>
#include <Engine/Service/EditorService.hpp>
#include <Engine/Service/WorldService.hpp>
#include <Engine/System/TransformSystem.hpp>
#include <Engine/System/RenderSystem.hpp>
#include <Engine/Registration.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <imgui_internal.h>

namespace
{

void DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue = 0.0f,
	float columnWidth = 100.0f)
{
	ImGuiIO& io = ImGui::GetIO();
	auto boldFont = io.Fonts->Fonts[0];

	ImGui::PushID(label.c_str());

	ImGui::Columns(2);
	ImGui::SetColumnWidth(0, columnWidth);
	ImGui::Text(label.c_str());
	ImGui::NextColumn();

	ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

	float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
	ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
	ImGui::PushFont(boldFont);
	if (ImGui::Button("X", buttonSize))
		values.x = resetValue;
	ImGui::PopFont();
	ImGui::PopStyleColor(3);

	ImGui::SameLine();
	ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
	ImGui::PopItemWidth();
	ImGui::SameLine();

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
	ImGui::PushFont(boldFont);
	if (ImGui::Button("Y", buttonSize))
		values.y = resetValue;
	ImGui::PopFont();
	ImGui::PopStyleColor(3);

	ImGui::SameLine();
	ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
	ImGui::PopItemWidth();
	ImGui::SameLine();

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
	ImGui::PushFont(boldFont);
	if (ImGui::Button("Z", buttonSize))
		values.z = resetValue;
	ImGui::PopFont();
	ImGui::PopStyleColor(3);

	ImGui::SameLine();
	ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
	ImGui::PopItemWidth();

	ImGui::PopStyleVar();

	ImGui::Columns(1);

	ImGui::PopID();
}

template <class T>
void DrawComponent(entt::entity entity, std::unique_ptr<engine::ecs::EntityManager>& em, std::function<void(T&)>&& uiFunction)
{
	ENGINE_ASSERT(engine::registration::helpers::typeRegistered<T>());

	const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed
		| ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap
		| ImGuiTreeNodeFlags_FramePadding;

	if (em->TryGetComponent<T>(entity))
	{
		const auto type = rttr::type::get<T>();
		const auto typeId = type.get_id();

		ImGui::PushID(&typeId);

		bool open = ImGui::TreeNodeEx(&typeId, treeNodeFlags, type.get_name().data());

		ImGui::SameLine();
		if (ImGui::Button("+"))
		{
			ImGui::OpenPopup("ComponentSettings");
		}

		bool removeComponent = false;
		if (ImGui::BeginPopup("ComponentSettings"))
		{
			if (ImGui::MenuItem("Remove component"))
				removeComponent = true;

			ImGui::EndPopup();
		}

		if (open)
		{
			uiFunction(em->GetComponent<T>(entity));
			ImGui::TreePop();
		}

		if (removeComponent)
		{
			em->RemoveComponent<T>(entity);
		}

		ImGui::PopID();
	}
}

} // unnamed

namespace engine::editor
{

void ComponentPanel::DrawPanel()
{
    auto& editorService = Instance().Service<EditorService>();
	auto& em = Instance().Service<WorldService>().CurrentWorld()->GetEntityManager();

    const auto selectedEntity = editorService.SelectedEntity();

    if (selectedEntity == C_INVALID_ENTITY)
    {
        return;
    }

	DrawComponent<TransformComponent>(selectedEntity, em, [](TransformComponent& t)
	{
		auto rotation = glm::degrees(glm::eulerAngles(t.m_rotation));

		DrawVec3Control("Position", t.m_position);
		DrawVec3Control("Rotation", rotation);
		DrawVec3Control("Scale", t.m_scale);

		t.m_rotation = glm::quat(glm::radians(rotation));
	});

	DrawComponent<DirectionalLightComponent>(selectedEntity, em, [](DirectionalLightComponent& l)
	{
		DrawVec3Control("Color", l.m_color);
		ImGui::Spacing();
		ImGui::DragFloat("Intensity", &l.m_intensity, 1, 0, 1000);
	});

	DrawComponent<MeshComponent>(selectedEntity, em, [](MeshComponent& l)
	{
		auto material = l.m_material->Material();

		if (auto materialBuffer = material->Buffer<PBRMaterialUB>(11))
		{
			bool dirty = false;
			ImGui::TextUnformatted("Albedo");

			if (ImGui::ColorPicker3("Color Picker", (float*)&materialBuffer->m_albedoVec))
			{
				dirty = true;
			}

			if (ImGui::Checkbox("Use albedo texture", &materialBuffer->m_useAlbedoTex))
			{
				dirty = true;
			}

			ImGui::Separator();

			ImGui::TextUnformatted("Roughness");
			if (ImGui::SliderFloat("Roughness", &materialBuffer->m_roughness, 0, 1))
			{
				dirty = true;
			}

			if (ImGui::Checkbox("Use roughness texture", &materialBuffer->m_useRoughnessTex))
			{
				dirty = true;
			}

			ImGui::Separator();

			ImGui::TextUnformatted("Metallic");
			if (ImGui::SliderFloat("Metallic", &materialBuffer->m_metallic, 0, 1))
			{
				dirty = true;
			}

			if (ImGui::Checkbox("Use metallic texture", &materialBuffer->m_useMetallicTex))
			{
				dirty = true;
			}

			if (dirty)
			{
				material->UpdateBuffer(11, *materialBuffer);
			}
		}
	});
}

} // engine::editor
