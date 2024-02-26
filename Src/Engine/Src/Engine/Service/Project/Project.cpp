#include <Engine/Service/Project/Project.hpp>
#include <Engine/Service/Filesystem/VirtualFilesystemService.hpp>
#include <Engine/Registration.hpp>
#include <nlohmann/json.hpp>
#include <fstream>

RTTR_REGISTRATION
{
    engine::registration::CommandLineArgs()
            .Argument(
                engine::registration::CommandLineArg("-prj", "--project")
                .Help("Absolute path to a project file")
                .DefaultValue("")
            );
}

namespace
{
constexpr std::string_view    C_TYPE_KEY = "__type__";
constexpr std::string_view    C_PROJECT_NAME_KEY = "name";
constexpr std::string_view    C_VERSION_KEY = "version";
constexpr std::string_view    C_SETTINGS_KEY = "settings";
constexpr uint8_t            C_VERSION = 0; // Please up project settings version each time you change the format
} // unnamed

namespace engine
{

using namespace nlohmann;

Project::Project(const io::fs::path& path)
{
    ENGINE_ASSERT(!path.empty());
    ENGINE_ASSERT(io::fs::exists(path));
    ENGINE_ASSERT(path.extension() == ".project");
    ENGINE_ASSERT(path.is_absolute());

    std::ifstream file(path);

    auto j = json::parse(file);
    m_name = j[C_PROJECT_NAME_KEY];
    m_version = j[C_VERSION_KEY];

    ENGINE_ASSERT(m_version == C_VERSION);

    // TODO: Implement smart settings parsing using reflection from RTTR

    auto& vfsSettingsJson = j[C_SETTINGS_KEY][0];
    ENGINE_ASSERT(vfsSettingsJson[C_TYPE_KEY] == "engine::io::VFSSettings");

    auto& vfsSettingsArray = vfsSettingsJson[C_SETTINGS_KEY];
    ENGINE_ASSERT(vfsSettingsArray.is_array());

    io::VFSSettings vfsSettings;
    for (auto& settingJson : vfsSettingsArray)
    {
        ENGINE_ASSERT(settingJson[C_TYPE_KEY] == "engine::io::VFSSettings::Setting");
        io::VFSSettings::Setting setting;
        setting.m_alias = settingJson["alias"];
        setting.m_path = settingJson["path"];
        vfsSettings.m_settings.emplace_back(std::move(setting));
    }

    m_settings.emplace_back(std::move(vfsSettings));
}

} // engine