#include "SceneSerializer.hpp"
#include "Path.hpp"
#include "Entity.hpp"
#include "AssetManager.hpp"
#include "MaterialLoader.hpp"
#include "Timer.hpp"
#include "Application.hpp"
#include "ThreadService.hpp"
#include "TextureLoader.hpp"
#include <taskflow/taskflow.hpp>
#include <taskflow/algorithm/for_each.hpp>
#include <fstream>

using namespace RightEngine;

namespace YAML
{
    template<>
    struct convert<glm::vec2>
    {
        static Node encode(const glm::vec2& rhs)
        {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.SetStyle(EmitterStyle::Flow);
            return node;
        }

        static bool decode(const Node& node, glm::vec2& rhs)
        {
            if (!node.IsSequence() || node.size() != 2)
                return false;

            rhs.x = node[0].as<float>();
            rhs.y = node[1].as<float>();
            return true;
        }
    };

    template<>
    struct convert<glm::vec3>
    {
        static Node encode(const glm::vec3& rhs)
        {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.push_back(rhs.z);
            node.SetStyle(EmitterStyle::Flow);
            return node;
        }

        static bool decode(const Node& node, glm::vec3& rhs)
        {
            if (!node.IsSequence() || node.size() != 3)
                return false;

            rhs.x = node[0].as<float>();
            rhs.y = node[1].as<float>();
            rhs.z = node[2].as<float>();
            return true;
        }
    };

    template<>
    struct convert<glm::vec4>
    {
        static Node encode(const glm::vec4& rhs)
        {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.push_back(rhs.z);
            node.push_back(rhs.w);
            node.SetStyle(EmitterStyle::Flow);
            return node;
        }

        static bool decode(const Node& node, glm::vec4& rhs)
        {
            if (!node.IsSequence() || node.size() != 4)
                return false;

            rhs.x = node[0].as<float>();
            rhs.y = node[1].as<float>();
            rhs.z = node[2].as<float>();
            rhs.w = node[3].as<float>();
            return true;
        }
    };

    template<>
    struct convert<xg::Guid>
    {
        static Node encode(const xg::Guid& uuid)
        {
            Node node;
            node.push_back(uuid.str());
            return node;
        }

        static bool decode(const Node& node, xg::Guid& uuid)
        {
            uuid = xg::Guid(node.as<std::string>());
            return true;
        }
    };

    Emitter& operator<<(YAML::Emitter& out, const glm::vec2& v)
    {
        out << YAML::Flow;
        out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
        return out;
    }

    Emitter& operator<<(YAML::Emitter& out, const glm::vec3& v)
    {
        out << YAML::Flow;
        out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
        return out;
    }

    Emitter& operator<<(YAML::Emitter& out, const glm::vec4& v)
    {
        out << YAML::Flow;
        out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
        return out;
    }
}

namespace
{
    void SerializeList(YAML::Emitter& output, const std::string& listName, std::function<void()> listFn)
    {
        output << YAML::Key << listName << YAML::Value << YAML::BeginMap;
        listFn();
        output << YAML::EndMap;
    }

    template<typename T>
    void SerializeKeyValue(YAML::Emitter& output, const std::string& key, const T& value)
    {
        output << YAML::Key << key << YAML::Value << value;
    }

    template<typename T>
    void SerializeComponent(YAML::Emitter& output,
                            const std::shared_ptr<Entity>& entity,
                            const std::string& componentName,
                            std::function<void(const T&)> componentCallback)
    {
        if (entity->HasComponent<T>())
        {
            output << YAML::Key << componentName;
            output << YAML::BeginMap;
            componentCallback(entity->GetComponent<T>());
            output << YAML::EndMap;
        }
    }

    std::string GetAssetTypeString(AssetType type)
    {
        switch (type)
        {
            case AssetType::IMAGE:
                return "Image";
            case AssetType::MESH:
                return "Mesh";
            case AssetType::ENVIRONMENT_MAP:
                return "Environment Map";
            case AssetType::SHADER:
                return "Shader";
            case AssetType::MATERIAL:
                return "Material";
            default:
            R_CORE_ASSERT(false, "")
        }
    }

    void SerializeAsset(YAML::Emitter& output, const xg::Guid& guid)
    {
        const auto assetPtr = AssetManager::Get().GetAsset<AssetBase>({ guid });
        R_CORE_ASSERT(assetPtr, "");

        output << YAML::BeginMap;
        output << YAML::Key << "Asset" << YAML::Value << YAML::BeginMap;
        SerializeKeyValue(output, "GUID", assetPtr->guid);
        SerializeKeyValue(output, "Path", assetPtr->path);
        SerializeKeyValue(output, "Type", static_cast<uint32_t>(assetPtr->type));
        if (assetPtr->type == AssetType::MATERIAL)
        {
            const auto materialPtr = AssetManager::Get().GetAsset<Material>({ guid });

            SerializeList(output, "Texture Data", [&]()
            {
                SerializeKeyValue(output, "Albedo GUID", materialPtr->textureData.albedo.guid);
                SerializeKeyValue(output, "Normal GUID", materialPtr->textureData.normal.guid);
                SerializeKeyValue(output, "Roughness GUID", materialPtr->textureData.roughness.guid);
                SerializeKeyValue(output, "Metallic GUID", materialPtr->textureData.metallic.guid);
                SerializeKeyValue(output, "AO GUID", materialPtr->textureData.ao.guid);
            });

            SerializeList(output, "Material Data", [&]()
            {
                SerializeKeyValue(output, "Albedo", materialPtr->materialData.albedo);
                SerializeKeyValue(output, "Roughness", materialPtr->materialData.roughness);
                SerializeKeyValue(output, "Metallic", materialPtr->materialData.metallic);
            });
        }
        output << YAML::EndMap;
        output << YAML::EndMap;
    }
}

SceneSerializer::SceneSerializer(const std::shared_ptr<Scene>& scene) : scene(scene)
{
}

bool SceneSerializer::Serialize(const fs::path& path)
{
    YAML::Emitter output;
    output << YAML::BeginMap;
    output << YAML::Key << "Scene" << YAML::Value << scene->GetName();
    output << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
    SerializeEntity(output, scene->GetRootNode());
    output << YAML::EndSeq;
    output << YAML::Key << "Assets" << YAML::Value << YAML::BeginSeq;
    SerializeAssets(output);
    output << YAML::EndSeq;
    output << YAML::EndMap;

    std::ofstream fout(path);
    fout << output.c_str();
    R_CORE_INFO("Serialized scene to {} successfully!", path.generic_u8string().c_str());
    return true;
}

bool SceneSerializer::Deserialize(const fs::path& path)
{
    Timer timer;
    YAML::Node data;
    try
    {
        data = YAML::LoadFile(path.generic_u8string());
    }
    catch (YAML::ParserException e)
    {
        R_CORE_ERROR("Failed to load scene file '{0}'\n     {1}", path.generic_u8string().c_str(), e.what());
        return false;
    }

    if (!data["Scene"])
        return false;

    std::string sceneName = data["Scene"].as<std::string>();
    R_CORE_TRACE("Deserializing scene '{0}'", sceneName);

    auto& ts = Instance().Service<ThreadService>();
    auto resourceLoadFuture = ts.AddBackgroundTask([&]()
    {
    	DeserializeAssets(data);
    });

    auto entities = data["Entities"];
    for (auto entityIt : entities)
    {
        auto entity = entityIt["Entity"];

		    auto tagComponent = entity["Tag component"];
		    auto name = tagComponent["Name"].as<std::string>();
		    auto tag = tagComponent["GUID"].as<std::string>();

		    auto entityPtr = scene->CreateEntityWithGuid(name, xg::Guid(tag), true);

		    auto transformComponent = entity["Transform component"];
		    R_CORE_ASSERT(transformComponent.IsDefined(), "");

		    auto& tc = entityPtr->GetComponent<TransformComponent>();
		    tc.position = transformComponent["Position"].as<glm::vec3>();
		    tc.rotation = transformComponent["Rotation"].as<glm::vec3>();
		    tc.scale = transformComponent["Scale"].as<glm::vec3>();

		    // TODO: Implement validation of all handles
		    auto& am = AssetManager::Get();
		    auto meshComponent = entity["Mesh component"];
		    if (meshComponent)
		    {
		        auto& mc = entityPtr->AddComponent<MeshComponent>();
		        mc.material = { meshComponent["Material GUID"].as<xg::Guid>() };
		        mc.mesh = { meshComponent["Mesh GUID"].as<xg::Guid>() };
		        mc.isVisible = { meshComponent["Is visible"].as<bool>() };
		    }

		    auto lightComponent = entity["Light component"];
		    if (lightComponent)
		    {
		        auto& lc = entityPtr->AddComponent<LightComponent>();
		        lc.type = static_cast<LightType>(lightComponent["Type"].as<uint32_t>());
		        lc.color = lightComponent["Color"].as<glm::vec3>();
		        lc.intensity = lightComponent["Intensity"].as<float>();
		        lc.outerRadius = lightComponent["Outer Radius"].as<float>();
		        lc.innerRadius = lightComponent["Inner Radius"].as<float>();
		    }

		    auto skyboxComponent = entity["Skybox component"];
		    if (skyboxComponent)
		    {
		        auto& sc = entityPtr->AddComponent<SkyboxComponent>();
		        sc.type = static_cast<SkyboxType>(skyboxComponent["Type"].as<uint32_t>());
		        sc.environmentHandle = { skyboxComponent["Skybox GUID"].as<xg::Guid>() };
		    }

		    auto cameraComponent = entity["Camera component"];
		    if (cameraComponent)
		    {
		        auto& cc = entityPtr->AddComponent<CameraComponent>();
		        cc.front = cameraComponent["Front"].as<glm::vec3>();
		        cc.worldUp = cameraComponent["World up"].as<glm::vec3>();
		        cc.up = cameraComponent["Up"].as<glm::vec3>();
		        cc.zNear = cameraComponent["Z near"].as<float>();
		        cc.zFar = cameraComponent["Z far"].as<float>();
		        cc.aspectRatio = cameraComponent["Aspect ratio"].as<float>();
		        cc.fov = cameraComponent["FOV"].as<float>();
		        cc.movementSpeed = cameraComponent["Movement speed"].as<float>();
		        cc.sensitivity = cameraComponent["Sensitivity"].as<float>();
		        cc.isActive = cameraComponent["Active"].as<bool>();
		        cc.isPrimary = cameraComponent["Primary"].as<bool>();
		    }
    }
    resourceLoadFuture.wait();
    R_CORE_INFO("Loaded scene {} successfully for {}s", path.generic_u8string().c_str(), timer.TimeInSeconds());
    return true;
}

void SceneSerializer::SerializeEntity(YAML::Emitter& output, const std::shared_ptr<Entity>& entity)
{
    R_CORE_ASSERT(entity->HasComponent<TagComponent>(), "");

    output << YAML::BeginMap;
    output << YAML::Key << "Entity" << YAML::Value << YAML::BeginMap;

    SerializeComponent<TagComponent>(output, entity, "Tag component", [&](const auto& component)
    {
        SerializeKeyValue(output, "GUID", component.guid);
        SerializeKeyValue(output, "Name", component.name);
    });

    SerializeComponent<TransformComponent>(output, entity, "Transform component", [&](const auto& component)
    {
        SerializeKeyValue(output, "Position", component.position);
        SerializeKeyValue(output, "Rotation", component.rotation);
        SerializeKeyValue(output, "Scale", component.scale);
    });

    SerializeComponent<MeshComponent>(output, entity, "Mesh component", [&](const auto& component)
    {
        SerializeKeyValue(output, "Mesh GUID", component.mesh.guid);
        SerializeKeyValue(output, "Material GUID", component.material.guid);
        SerializeKeyValue(output, "Is visible", component.isVisible);
        sceneAssets.insert(component.mesh.guid);
        SaveMaterial(component.material);
    });

    SerializeComponent<LightComponent>(output, entity, "Light component", [&](const auto& component)
    {
        SerializeKeyValue(output, "Type", static_cast<int>(component.type));
        SerializeKeyValue(output, "Color", component.color);
        SerializeKeyValue(output, "Intensity", component.intensity);
        SerializeKeyValue(output, "Outer Radius", component.outerRadius);
        SerializeKeyValue(output, "Inner Radius", component.innerRadius);
    });

    SerializeComponent<SkyboxComponent>(output, entity, "Skybox component", [&](const auto& component)
    {
        SerializeKeyValue(output, "Type", static_cast<int>(component.type));
        SerializeKeyValue(output, "Skybox GUID", component.environmentHandle.guid);
        sceneAssets.insert(component.environmentHandle.guid);
    });

    SerializeComponent<CameraComponent>(output, entity, "Camera component", [&](const auto& component)
    {
        SerializeKeyValue(output, "Front", component.front);
        SerializeKeyValue(output, "World up", component.worldUp);
        SerializeKeyValue(output, "Up", component.up);
        SerializeKeyValue(output, "Z near", component.zNear);
        SerializeKeyValue(output, "Z far", component.zFar);
        SerializeKeyValue(output, "Aspect ratio", component.aspectRatio);
        SerializeKeyValue(output, "FOV", component.GetFOV());
        SerializeKeyValue(output, "Movement speed", component.movementSpeed);
        SerializeKeyValue(output, "Sensitivity", component.sensitivity);
        SerializeKeyValue(output, "Active", component.isActive);
        SerializeKeyValue(output, "Primary", component.isPrimary);
    });

    output << YAML::EndMap;
    output << YAML::EndMap;

    for (const auto& child : entity->GetChildren())
    {
        SerializeEntity(output, child);
    }
}

void SceneSerializer::SerializeAssets(YAML::Emitter& output)
{
    for (auto& guid : sceneAssets)
    {
        SerializeAsset(output, guid);
    }
}

void SceneSerializer::SaveMaterial(const AssetHandle& handle)
{
    const auto assetPtr = AssetManager::Get().GetAsset<Material>(handle);
    R_CORE_ASSERT(assetPtr, "");

    sceneAssets.insert(assetPtr->textureData.albedo.guid);
    sceneAssets.insert(assetPtr->textureData.normal.guid);
    sceneAssets.insert(assetPtr->textureData.roughness.guid);
    sceneAssets.insert(assetPtr->textureData.metallic.guid);
    sceneAssets.insert(assetPtr->textureData.ao.guid);

    sceneAssets.insert(handle.guid);
}

void SceneSerializer::LoadDependencies(const std::vector<std::shared_ptr<AssetDependency>>& assetDependencies)
{
    tf::Taskflow taskflow;
    auto& am = AssetManager::Get();
    taskflow.for_each(
        assetDependencies.begin(), assetDependencies.end(), [&am](const auto dep)
        {
            switch (dep->type)
            {
            case AssetType::MESH:
            {
                auto loader = am.GetLoader<MeshLoader>();
                loader->LoadWithGUID(dep->path, dep->guid);
                break;
            }
            case AssetType::ENVIRONMENT_MAP:
            {
                auto loader = am.GetLoader<EnvironmentMapLoader>();
                loader->LoadWithGUID(dep->path, dep->guid);
                break;
            }
            case AssetType::SHADER:
                break;
            case AssetType::IMAGE:
            {
                auto loader = am.GetLoader<TextureLoader>();
                loader->LoadWithGUID(dep->path, {}, dep->guid);
                break;
            }
            case AssetType::MATERIAL:
            {
                break;
            }
            default:
                R_CORE_ASSERT(false, "")
            }
        }
    );

    Instance().Service<ThreadService>().AddBackgroundTaskflow(std::move(taskflow)).wait();

    // We must load materials only after other resources was load to be sure that all default ones will be properly set up
    for (const auto dep : assetDependencies)
    {
        if (dep->type == AssetType::MATERIAL)
        {
            auto loader = am.GetLoader<MaterialLoader>();
            const auto ah = loader->LoadWithGUID(dep->guid);
            auto material = am.GetAsset<Material>(ah);
            R_CORE_ASSERT(material, "");
            auto materialDep = std::static_pointer_cast<MaterialAssetDependency>(dep);
            R_CORE_ASSERT(materialDep, "");
            material->textureData.albedo = { materialDep->albedoGuid };
            material->textureData.normal = { materialDep->normalGuid };
            material->textureData.roughness = { materialDep->roughnessGuid };
            material->textureData.metallic = { materialDep->metallicGuid };
            material->textureData.ao = { materialDep->aoGuid };
            material->materialData.albedo = materialDep->albedo;
            material->materialData.metallic = materialDep->metallic;
            material->materialData.roughness = materialDep->roughness;
        }
    }
}

void SceneSerializer::DeserializeAssets(YAML::Node& node)
{
    auto assets = node["Assets"];
    std::vector<std::shared_ptr<AssetDependency>> dependencies;

    for (auto assetIt : assets)
    {
        auto asset = assetIt["Asset"];

        AssetType type = static_cast<AssetType>(asset["Type"].as<uint32_t>());
        std::shared_ptr<AssetDependency> dependency;
        if (type == AssetType::MATERIAL)
        {
            dependency = std::make_shared<MaterialAssetDependency>();
        }
        else
        {
            dependency = std::make_shared<AssetDependency>();
        }

        dependency->type = type;
        dependency->guid = asset["GUID"].as<xg::Guid>();
        dependency->path = asset["Path"].as<std::string>();

        if (type == AssetType::MATERIAL)
        {
            auto materialDependency = std::static_pointer_cast<MaterialAssetDependency>(dependency);
            R_CORE_ASSERT(materialDependency, "");
            auto textureData = asset["Texture Data"];
            materialDependency->albedoGuid = textureData["Albedo GUID"].as<xg::Guid>();
            materialDependency->normalGuid = textureData["Normal GUID"].as<xg::Guid>();
            materialDependency->roughnessGuid = textureData["Roughness GUID"].as<xg::Guid>();
            materialDependency->metallicGuid = textureData["Metallic GUID"].as<xg::Guid>();
            materialDependency->aoGuid = textureData["AO GUID"].as<xg::Guid>();

            auto materialData = asset["Material Data"];
            materialDependency->albedo = materialData["Albedo"].as<glm::vec4>();
            materialDependency->metallic = materialData["Metallic"].as<float>();
            materialDependency->roughness = materialData["Roughness"].as<float>();
        }

        dependencies.push_back(dependency);
    }

    LoadDependencies(dependencies);
}
