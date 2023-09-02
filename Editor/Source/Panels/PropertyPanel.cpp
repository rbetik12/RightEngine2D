#include "PropertyPanel.hpp"
#include "Components.hpp"
#include "String.hpp"
#include "Path.hpp"
#include "AssetManager.hpp"
#include "ImGuiLayer.hpp"
#include "Filesystem.hpp"
#include "EditorCore.hpp"
#include "TextureLoader.hpp"
#include "Application.hpp"
#include <Engine/Service/ThreadService.hpp>
#include <imgui.h>
#include <imgui_internal.h>
#include <filesystem>

using namespace RightEngine;

namespace
{
	AssetHandle editorDefaultTexture;
	const std::string texturesDir = "/Textures/";
	const auto modelsDir = "/Models/";
	bool isDisplayCalled = false;

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
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{0, 0});

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = {lineHeight + 3.0f, lineHeight};

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.8f, 0.1f, 0.15f, 1.0f});
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.9f, 0.2f, 0.2f, 1.0f});
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.8f, 0.1f, 0.15f, 1.0f});
		ImGui::PushFont(boldFont);
		if (ImGui::Button("X", buttonSize))
			values.x = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.2f, 0.7f, 0.2f, 1.0f});
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.3f, 0.8f, 0.3f, 1.0f});
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.2f, 0.7f, 0.2f, 1.0f});
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Y", buttonSize))
			values.y = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.1f, 0.25f, 0.8f, 1.0f});
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.2f, 0.35f, 0.9f, 1.0f});
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.1f, 0.25f, 0.8f, 1.0f});
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

	std::string CopyFileToAssetDirectory(const std::string& filePath, const std::string& assetDirectory)
	{
		auto filename = String::Split(filePath, "/").back();
		const std::filesystem::path from = filePath;
		const std::filesystem::path to = Path::Absolute("/Assets/Textures/") + filename;
		R_CORE_TRACE("{0}", filename);
		try
		{
			std::filesystem::copy(from, to, std::filesystem::copy_options::update_existing);
		}
		catch (std::exception& ex)
		{
			R_CORE_ASSERT(false, ex.what());
		}

		return filename;
	}

	std::string OpenFileDialogOnItemClick(const std::string& title,
	                                      const std::vector<std::string>& filters,
	                                      const std::string& assetDir)
	{
		if (ImGui::IsItemClicked())
		{
			const auto filepath = Filesystem::OpenFileDialog(filters).string();
			return CopyFileToAssetDirectory(filepath, assetDir);
		}

		return "";
	}

	void DrawMaterialEditorTab(const std::string& label,
	                           bool hasTexture,
	                           AssetHandle& textureHandle,
	                           std::unordered_map<std::string, AssetHandle>& textures,
	                           MeshComponent& component)
	{
		auto& assetManager = AssetManager::Get();
		ImGui::LabelText("", label.c_str());
		if (hasTexture)
		{
			ImGuiLayer::Image(assetManager.GetAsset<Texture>(textureHandle), ImVec2(64, 64), ImVec2(0, 1),
			                  ImVec2(1, 0));
		}
		else
		{
			ImGuiLayer::Image(assetManager.GetAsset<Texture>(editorDefaultTexture), ImVec2(64, 64));
		}

		if (ImGui::IsItemClicked(1))
		{
			textureHandle = AssetManager::Get().GetDefaultTexture();
			return;
		}

		if (ImGui::BeginDragDropTarget())
		{
			const auto payload = ImGui::AcceptDragDropPayload(editor::C_CONTENT_BROWSER_DND_NAME);
			if (!payload)
			{
				ImGui::EndDragDropTarget();
				return;
			}
			static char pathBuff[256]{};
			memset(pathBuff, 0, 256);
			memcpy(pathBuff, payload->Data, payload->DataSize);
			fs::path path = pathBuff;
			if (!path.has_extension())
			{
				ImGui::EndDragDropTarget();
				return;
			}
			if (path.extension() == ".png" || path.extension() == ".jpg")
			{
				auto& ts = Instance().Service<engine::ThreadService>();
				ts.AddBackgroundTask([&textureHandle, &assetManager, path = std::move(path)]()
					{
						textureHandle = assetManager.GetLoader<TextureLoader>()->Load(path.generic_string());
					});
			}
			ImGui::EndDragDropTarget();
		}
		
	}

	std::string LightTypeToStr(LightType type)
	{
		switch (type)
		{
		case LightType::DIRECTIONAL:
			return "Directional";
		case LightType::POINT:
			return "Point";
		default:
			R_CORE_ASSERT(false, "")
		}
	}
}

namespace editor
{
	PropertyPanel::PropertyPanel(const std::shared_ptr<Scene>& aScene)
	{
		scene = aScene;
	}

	void PropertyPanel::Init()
	{
		auto& ts = Instance().Service<engine::ThreadService>();
		ts.AddBackgroundTask([=]()
			{
				editorDefaultTexture = AssetManager::Get().GetLoader<TextureLoader>()->Load(
					"/Textures/editor_default_texture.png");
				AssetManager::Get().GetAsset<Texture>(editorDefaultTexture)->SetSampler(Device::Get()->CreateSampler({}));
			});
	}

	void PropertyPanel::SetScene(const std::shared_ptr<Scene>& aScene)
	{
		scene = aScene;
	}

	void PropertyPanel::SetSelectedEntity(const std::shared_ptr<Entity>& entity)
	{
		selectedEntity = entity;
	}

	template <class T>
	void DrawComponent(const std::string& componentName, const std::shared_ptr<Entity>& entity,
	                   std::function<void(T&)> uiFunction)
	{
		const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed
			| ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap
			| ImGuiTreeNodeFlags_FramePadding;
		if (entity->HasComponent<T>())
		{
			auto& component = entity->GetComponent<T>();
			bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, componentName.c_str());

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
				uiFunction(component);
				ImGui::TreePop();
			}

			if (removeComponent)
			{
				entity->RemoveComponent<T>();
			}
		}
	}

	void PropertyPanel::OnImGuiRender()
	{
		isDisplayCalled = false;
		ImGui::Begin("Properties");
		if (selectedEntity)
		{
			if (ImGui::Button("Add Component"))
				ImGui::OpenPopup("AddComponent");

			if (ImGui::BeginPopup("AddComponent"))
			{
				DisplayAddComponentEntry<MeshComponent>("Mesh");
				DisplayAddComponentEntry<CameraComponent>("Camera");
				DisplayAddComponentEntry<LightComponent>("Light");

				ImGui::EndPopup();
			}

			DrawComponent<TagComponent>("Tag", selectedEntity, [](auto& component)
			{
				const size_t bufSize = 256;
				char buf[bufSize];
				R_CORE_ASSERT(component.name.size() < bufSize, "");
				snprintf(buf, bufSize, component.name.c_str());
				ImGui::InputText("Entity Name", buf, bufSize);
				//TODO: Change name only if buf was changed
				component.name = std::string(buf);
				ImGui::Separator();
				ImGui::Separator();
				ImGui::LabelText("GUID", "%s", component.guid.str().c_str());
			});

			DrawComponent<TransformComponent>("Transform", selectedEntity, [](auto& component)
			{
				auto& position = component.position;
				DrawVec3Control("Position", position);
				auto rotation = glm::degrees(component.rotation);
				DrawVec3Control("Rotation", rotation);
				if (rotation != component.rotation)
				{
					component.SetRotationDegree(rotation);
				}
				auto& scale = component.scale;
				DrawVec3Control("Scale", scale);
			});

			DrawComponent<SkyboxComponent>("Skybox", selectedEntity, [this](auto& component)
			{
				auto& assetManager = AssetManager::Get();
				const auto& currentEnvironment = assetManager.GetAsset<EnvironmentContext>(component.environmentHandle);
				ImGui::LabelText("Image name", "%s", currentEnvironment->name.c_str());
				ImGui::Separator();
				ImGuiLayer::Image(currentEnvironment->equirectangularTexture, ImVec2(512, 256), ImVec2(0, 1),
				                  ImVec2(1, 0));

				if (ImGui::BeginDragDropTarget())
				{
					const auto payload = ImGui::AcceptDragDropPayload(C_CONTENT_BROWSER_DND_NAME);
					if (!payload)
					{
						ImGui::EndDragDropTarget();
						return;
					}
					static char pathBuff[256]{};
					memset(pathBuff, 0, 256);
					memcpy(pathBuff, payload->Data, payload->DataSize);
					fs::path path = pathBuff;
					if (!path.has_extension())
					{
						ImGui::EndDragDropTarget();
						return;
					}
					if (path.extension() == ".hdr")
					{
						component.environmentHandle = assetManager.GetLoader<EnvironmentMapLoader>()->Load(path.generic_string());
					}
					ImGui::EndDragDropTarget();
				}
			});

			DrawComponent<MeshComponent>("Mesh", selectedEntity, [this](auto& component)
			{
				auto& assetManager = AssetManager::Get();
				ImGui::LabelText("Mesh GUID", "%s", component.mesh.guid.str().c_str());
				if (ImGui::BeginDragDropTarget())
				{
					const auto payload = ImGui::AcceptDragDropPayload(C_CONTENT_BROWSER_DND_NAME);
					if (!payload)
					{
						ImGui::EndDragDropTarget();
						return;
					}
					static char pathBuff[256]{};
					memset(pathBuff, 0, 256);
					memcpy(pathBuff, payload->Data, payload->DataSize);
					fs::path path = pathBuff;
					if (!path.has_extension())
					{
						ImGui::EndDragDropTarget();
						return;
					}
					if (path.extension() == ".obj" || path.extension() == ".gltf" || path.extension() == ".fbx")
					{
						component.mesh = assetManager.GetLoader<MeshLoader>()->Load(path.generic_string());
					}
					ImGui::EndDragDropTarget();
				}
				ImGui::Separator();

				bool isVisible = component.isVisible;
				ImGui::Checkbox("Is visible", &isVisible);
				component.isVisible = isVisible;
				ImGui::Separator();

				if (ImGui::BeginTable("split", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings))
				{
					auto& materialRef = component.material;
					auto material = AssetManager::Get().GetAsset<Material>(materialRef);
					ImGui::TableNextColumn();
					DrawMaterialEditorTab("Albedo", true, material->textureData.albedo, textures, component);
					ImGui::TableNextColumn();
					ImGui::ColorEdit4("", &material->materialData.albedo.x);

					ImGui::TableNextColumn();
					DrawMaterialEditorTab("Normal", true, material->textureData.normal, textures, component);
					ImGui::TableNextColumn();

					ImGui::TableNextColumn();
					DrawMaterialEditorTab("Metallic", true, material->textureData.metallic, textures, component);
					ImGui::TableNextColumn();
					ImGui::SliderFloat("Metallic", &material->materialData.metallic,0, 1);

					ImGui::TableNextColumn();
					DrawMaterialEditorTab("Roughness", true, material->textureData.roughness, textures, component);
					ImGui::TableNextColumn();
					ImGui::SliderFloat("Roughness", &material->materialData.roughness,0, 1);

					ImGui::TableNextColumn();
					DrawMaterialEditorTab("AO", true, material->textureData.ao, textures, component);
					ImGui::TableNextColumn();


					ImGui::EndTable();
				}
			});

			DrawComponent<CameraComponent>("Camera", selectedEntity, [this](auto& component)
			{
				ImGui::Checkbox("Is primary", &component.isPrimary);

				ImGui::SliderFloat("Movement speed", &component.movementSpeed, 40.0f, 150.0f);

				ImGui::SliderFloat("FOV", &component.fov, 30.0f, 100.0f);

				ImGui::SliderFloat("Z Near", &component.zNear, 0.1f, 1.0f);

				ImGui::SliderFloat("Z Far", &component.zFar, 10.0f, 1000.0f);

				std::array<const char*, 3> aspectRatios = {"16/9", "4/3", "Fit to window"};
				static const char* currentRatio = aspectRatios[2];
				if (ImGui::BeginCombo("Aspect ratio", currentRatio))
				{
					for (int i = 0; i < aspectRatios.size(); i++)
					{
						bool isSelected = (currentRatio == aspectRatios[i]);
						if (ImGui::Selectable(aspectRatios[i], isSelected))
						{
							currentRatio = aspectRatios[i];
						}
						if (isSelected)
						{
							ImGui::SetItemDefaultFocus();
						}
					}
					ImGui::EndCombo();
				}
				if (currentRatio)
				{
					float newAspectRatio;
					if (std::strcmp(currentRatio, aspectRatios[0]) == 0)
					{
						newAspectRatio = 16.0f / 9.0f;
					}
					if (std::strcmp(currentRatio, aspectRatios[1]) == 0)
					{
						newAspectRatio = 4.0f / 3.0f;
					}
					if (std::strcmp(currentRatio, aspectRatios[2]) == 0)
					{
						newAspectRatio = component.aspectRatio;
					}
					component.aspectRatio = newAspectRatio;
				}
			});

			DrawComponent<LightComponent>("Light", selectedEntity, [this](auto& component)
			{
				std::array<LightType, 2> lightTypes = {LightType::DIRECTIONAL, LightType::POINT};
				if (ImGui::BeginCombo("Light type", LightTypeToStr(component.type).c_str()))
				{
					for (auto& type : lightTypes)
					{
						bool isSelected = component.type == type;
						if (ImGui::Selectable(LightTypeToStr(type).c_str(), isSelected))
						{
							component.type = type;
						}

						if (isSelected)
						{
							ImGui::SetItemDefaultFocus();
						}
					}
					ImGui::EndCombo();
				}

				ImGui::Separator();

				glm::vec3 color = component.color * 255.0f;
				DrawVec3Control("Color", color);
				component.color = glm::clamp(color / 255.0f, 0.0f, 1.0f);

				ImGui::Separator();

				ImGui::SliderFloat("Intensity", &component.intensity, 0.0f, 100.0f);
				ImGui::SliderFloat("Outer radius", &component.outerRadius, 0.0f, 200.0f);
				ImGui::SliderFloat("Inner radius", &component.innerRadius, 0.0f, 200.0f);
			});
		}
		ImGui::End();
	}
}
