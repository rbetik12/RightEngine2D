#include "Path.hpp"
#include "Core.hpp"
#include <filesystem>
#include <sstream>

using namespace RightEngine;

std::unordered_map<std::string, std::string> Path::aliasMap;
bool Path::initialized = false;

void Path::Init()
{
    R_CORE_ASSERT(!initialized, "");
    initialized = true;
    aliasMap = {
        { "/Engine", G_ENGINE_ASSET_DIR },
        { "/Config", G_CONFIG_DIR },
        { "/", G_ASSET_DIR },
    };
}

std::string Path::Absolute(const std::string& enginePath)
{
    std::string fullPath;
    if (enginePath == "/Config/imgui.ini")
    {
        int kek;
    }
    for (const auto& [alias, path] : aliasMap)
    {
	    if (enginePath.rfind(alias, 0) == 0)
	    {
            fullPath.append(enginePath.substr(alias.size()));
            R_CORE_ASSERT(!fullPath.empty(), "");
            if (fullPath.at(0) != '/')
            {
                fullPath = '/' + fullPath;
            }
            fullPath = path + fullPath;
            break;
	    }
    }
    R_CORE_ASSERT(!fullPath.empty(), "");
    return fullPath;
}

std::string Path::Engine(const std::string& absolutePath)
{
    std::string enginePath;
    for (const auto& [alias, path] : aliasMap)
    {
        if (absolutePath.rfind(path, 0) == 0)
        {
            const auto kek = absolutePath.substr(path.size());
            enginePath.append(alias).append(absolutePath.substr(path.size()));
            if (enginePath.find("//") != std::string::npos)
            {
                enginePath.replace(0, 2, "/");
            }
            break;
        }
    }
    return enginePath;
}
