#pragma once

#include "Shader.hpp"
#include "Device.hpp"
#include <glslang/Include/glslang_c_interface.h>
#include <vulkan/vulkan.h>

namespace RightEngine
{
    enum class ShaderModuleType
    {
        VERTEX,
        FRAGMENT
    };
    struct ShaderProgramSource
    {
        std::string vertexSource;
        std::string fragmentSource;
    };

    class VulkanShader : public Shader
    {
    public:
        VulkanShader(const std::shared_ptr<Device>& device, const ShaderProgramDescriptor& descriptor);
        virtual ~VulkanShader() override;

        virtual void Bind() const override;
        virtual void UnBind() const override;

        const VkShaderModule& GetShaderModule(ShaderModuleType type) const
        {
            switch (type)
            {
                case ShaderModuleType::VERTEX:
                    return vertexShaderModule;
                case ShaderModuleType::FRAGMENT:
                    return fragShaderModule;
            }
        }

        VkVertexInputBindingDescription GetVertexFormatDescription() const;
        std::vector<VkVertexInputAttributeDescription> GetVertexFormatAttribute() const;

    private:
        VkShaderModule vertexShaderModule;
        VkShaderModule fragShaderModule;
    };
}