#include "VulkanShaderCompiler.hpp"
#include <fstream>
#include <sstream>

namespace rhi::vulkan
{
	ShaderData VulkanShaderCompiler::Compile(std::string_view path)
	{
        core::log::debug("{}", path);
		return {};
	}
 
	std::string VulkanShaderCompiler::ReadShader(std::string_view path) const
	{
        core::log::debug("{}", path);
        // std::ifstream stream(std::string{ path });
        //
        // if (!stream.is_open())
        // {
        //     std::cout
        // }
        //
        // std::string line;
        // std::stringstream ss[2];
        //
        // while (std::getline(vertexShaderStream, line))
        // {
        //     ss[0] << line << '\n';
        // }
        //
        // while (std::getline(fragmentShaderStream, line))
        // {
        //     ss[1] << line << '\n';
        // }
        //
        // return { ss[0].str(), ss[1].str() };
        return {};
	}
}
