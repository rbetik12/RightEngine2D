#pragma once

#include <Engine/Config.hpp>
#include <Core/RTTRIntegration.hpp>
#include <filesystem>

namespace engine
{

namespace io
{
namespace fs = std::filesystem;
} // io

class ENGINE_API Project
{
public:
    // Absolute path to a project file
    Project(const io::fs::path& path);

    template<typename T>
    T& Setting()
    {
        auto it = eastl::find_if(m_settings.begin(), m_settings.end(), [](const rttr::variant& variant)
            {
                return variant.get_type() == rttr::type::get<T>();
            });

        ENGINE_ASSERT(it != m_settings.end());

        return it->template get_value<T>();
    }

private:
    std::string                        m_name;
    uint8_t                            m_version;
    eastl::vector<rttr::variant>    m_settings;
};

} // engine