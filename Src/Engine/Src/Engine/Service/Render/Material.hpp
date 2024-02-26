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

    template<typename T>
    inline void SetBuffer(T& bufferObject, int slot, rhi::ShaderStage stage, std::string_view name = "", int offset = 0)
    {
        ENGINE_ASSERT(registration::helpers::typeRegistered<T>());
        ENGINE_ASSERT(slot < m_buffers.size());
        ENGINE_ASSERT(stage != rhi::ShaderStage::NONE);

        auto& rs = Instance().Service<RenderService>();

        BufferInfo buffer{};
        buffer.m_cpuBuffer = bufferObject;
        buffer.m_offset = offset;
        buffer.m_stage = stage;

        rhi::BufferDescriptor descriptor{};
        descriptor.m_size = sizeof(T);
        descriptor.m_memoryType = rhi::MemoryType::CPU_GPU;
        descriptor.m_type = rhi::BufferType::UNIFORM;
        descriptor.m_name = name;

        buffer.m_gpuBuffer = rs.CreateBuffer(descriptor);
        buffer.m_gpuBuffer->CopyToBuffer(&bufferObject, sizeof(T));

        m_pendingBuffers.emplace_back(slot, std::move(buffer));
        m_dirty = true;
    }

    void SetTexture(const std::shared_ptr<rhi::Texture>& texture, int slot);

    void Sync();

private:
    struct BufferInfo
    {
        rttr::variant                    m_cpuBuffer;
        std::shared_ptr<rhi::Buffer>    m_gpuBuffer;
        int                                m_offset = 0;
        rhi::ShaderStage                m_stage = rhi::ShaderStage::NONE;
    };

    struct TextureInfo
    {
        std::shared_ptr<rhi::Texture> m_texture;
    };

    eastl::vector<BufferInfo>                        m_buffers;
    eastl::vector<TextureInfo>                        m_textures;
    eastl::vector<eastl::pair<int, BufferInfo>>        m_pendingBuffers;
    eastl::vector<eastl::pair<int, TextureInfo>>    m_pendingTextures;
    std::shared_ptr<rhi::Shader>                    m_shader;
    bool                                            m_dirty;
};

} // engine::render