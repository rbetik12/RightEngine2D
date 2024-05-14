#pragma once

#include <Engine/Config.hpp>
#include <Engine/Engine.hpp>
#include <Engine/Registration.hpp>
#include <Engine/Service/Render/RenderService.hpp>
#include <Core/RTTRIntegration.hpp>
#include <RHI/Buffer.hpp>
#include <RHI/ShaderDescriptor.hpp>
#include <RHI/Texture.hpp>
#include <RHI/Shader.hpp>

namespace engine::render
{

class ENGINE_API Material
{
public:
    Material(const std::shared_ptr<rhi::Shader>& shader);

    const std::shared_ptr<rhi::Shader>& Shader() { return m_shader; }
    const std::shared_ptr<rhi::GPUMaterial>& GPUMaterial() { return m_gpuMaterial; }

    template<typename T>
    inline void SetBuffer(int slot, rhi::ShaderStage stage, std::string_view name = "", int offset = 0)
    {
        ENGINE_ASSERT(registration::helpers::typeRegistered<T>());

        SetBuffer(rttr::type::get<T>(), slot, stage, name, offset);
    }

    template <typename T>
    inline void UpdateBuffer(int slot, const T& bufferObject)
    {
        ENGINE_ASSERT(registration::helpers::typeRegistered<T>());

        auto& info = m_buffers[slot];

        ENGINE_ASSERT(info.m_cpuBuffer.is_valid());
        const auto name = info.m_cpuBuffer.get_type().get_name();
        const auto name2 = rttr::type::get<T>().get_name();
        ENGINE_ASSERT(name == name2);

        info.m_cpuBuffer = bufferObject;
        info.m_gpuBuffer->CopyToBuffer(&bufferObject, sizeof(T));
    }

    void SetBuffer(rttr::type type, int slot, rhi::ShaderStage stage, std::string_view name = "", int offset = 0);

    void SetTexture(const std::shared_ptr<rhi::Texture>& texture, int slot);

    void Sync();

private:
    struct BufferInfo
    {
        rttr::variant                   m_cpuBuffer;
        std::shared_ptr<rhi::Buffer>    m_gpuBuffer;
        int                             m_offset = 0;
        rhi::ShaderStage                m_stage = rhi::ShaderStage::NONE;
    };

    struct TextureInfo
    {
        std::shared_ptr<rhi::Texture> m_texture;
    };

    eastl::vector<BufferInfo>                        m_buffers;
    eastl::vector<TextureInfo>                       m_textures;
    eastl::vector<eastl::pair<uint8_t, BufferInfo>>  m_pendingBuffers;
    eastl::vector<eastl::pair<uint8_t, TextureInfo>> m_pendingTextures;
    std::shared_ptr<rhi::Shader>                     m_shader;
    std::shared_ptr<rhi::GPUMaterial>                m_gpuMaterial;
    bool                                             m_dirty;
};

} // engine::render