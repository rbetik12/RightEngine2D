#pragma once

#include "AssetBase.hpp"
#include "Types.hpp"
#include "VertexBufferLayout.hpp"
#include "Device.hpp"
#include "ShaderProgramDescriptor.hpp"
#include <glm/matrix.hpp>
#include <string>
#include <memory>
#include <unordered_map>

namespace RightEngine
{
    class Shader : public AssetBase
    {
    public:
        ASSET_BASE()

        virtual ~Shader() = default;

        virtual void Bind() const = 0;
        virtual void UnBind() const = 0;

        inline const ShaderProgramDescriptor& GetShaderProgramDescriptor() const
        { return descriptor; }

    protected:
        Shader(const std::shared_ptr<Device>& device, const ShaderProgramDescriptor& descriptor) : descriptor(descriptor)
        {}

        ShaderProgramDescriptor descriptor;
    };
}