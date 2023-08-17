#include "VulkanShaderCompiler.hpp"

#include <filesystem>
#include <fmt/format.h>
#include <fstream>
#include <sstream>

namespace fs = std::filesystem;

namespace rhi::vulkan
{
	ShaderData VulkanShaderCompiler::Compile(std::string_view path)
	{
        rhi::log::info("Compiling {}", path);
        const auto rawShaderStr = ReadShader(path);
        if (rawShaderStr.empty())
        {
            return {};
        }

        const auto processedShaderStr = PreprocessShader(rawShaderStr, path);
        if (processedShaderStr.empty())
        {
            return {};
        }

        rhi::log::debug("{}", processedShaderStr);

		return {};
	}
 
	std::string VulkanShaderCompiler::ReadShader(std::string_view path) const
	{
        std::ifstream stream(std::string{ path });
        
        if (!stream.is_open())
        {
            rhi::log::error("Could not find shader file: {}", path);
            return "";
        }

        std::string line;
        std::stringstream ss;

        while (std::getline(stream, line))
        {
            ss << line << '\n';
        }

        return ss.str();
	}

	std::string VulkanShaderCompiler::PreprocessShader(std::string_view shaderCode, std::string_view path) const
	{
        std::istringstream stream(std::string{ shaderCode });
        std::string line;
        std::stringstream processedSource;

        while (std::getline(stream, line)) 
        {
            if (line.find("#include") != std::string::npos) 
            {
                size_t start = line.find("\"");
                size_t end = line.find("\"", start + 1);
                if (start != std::string::npos && end != std::string::npos) 
                {
                    const auto shaderDir = fs::path(path).parent_path().generic_u8string();
                    std::string includePath = fmt::format("{}/{}", shaderDir, line.substr(start + 1, end - start - 1));
                    std::string includedContent = ReadShader(includePath);
                    if (includedContent.empty())
                    {
                        return "";
                    }
                    processedSource << includedContent << "\n";
                }
            }
            else 
            {
                processedSource << line << "\n";
            }
        }

        return processedSource.str();
	}
}
