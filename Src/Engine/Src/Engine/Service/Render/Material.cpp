#include <Engine/Service/Render/Material.hpp>
#include <Engine/Service/Render/RenderService.hpp>
#include <RHI/GPUMaterial.hpp>

namespace
{
constexpr int C_MAX_SHADER_BUFFER_AMOUNT = 16;
constexpr int C_MAX_SHADER_TEXTURE_AMOUNT = 16;
} // unnamed

namespace engine::render
{

Material::Material(const std::shared_ptr<rhi::Shader>& shader) : m_shader(shader)
{
    m_buffers.resize(C_MAX_SHADER_BUFFER_AMOUNT);
    m_textures.resize(C_MAX_SHADER_TEXTURE_AMOUNT);

    const auto data = s_storage.TexData(m_shader);

    if (!data.empty())
    {
        for (const auto& tex : data)
        {
            if (!tex.m_texture.expired())
            {
                SetTexture(tex.m_texture.lock(), tex.m_slot);
            }
            else
            {
                ENGINE_ASSERT(false);
            }
        }

        Sync();
    }
}

void Material::SetBuffer(rttr::type type, int slot, rhi::ShaderStage stage, std::string_view name, int offset)
{
    ENGINE_ASSERT(type.is_valid());
    ENGINE_ASSERT(slot < m_buffers.size());
    ENGINE_ASSERT(stage != rhi::ShaderStage::NONE);

    auto& rs = Instance().Service<RenderService>();

    BufferInfo buffer{};
    buffer.m_cpuBuffer = type.create();
    buffer.m_offset = offset;
    buffer.m_stage = stage;

    rhi::BufferDescriptor descriptor{};
    descriptor.m_size = static_cast<uint32_t>(type.get_sizeof());
    descriptor.m_memoryType = rhi::MemoryType::CPU_GPU;
    descriptor.m_type = rhi::BufferType::UNIFORM;
    descriptor.m_name = name;

    buffer.m_gpuBuffer = rs.CreateBuffer(descriptor);

    m_pendingBuffers.emplace_back(slot, std::move(buffer));
    m_dirty = true;
}

void Material::SetTexture(const std::shared_ptr<rhi::Texture>& texture, uint8_t slot, uint8_t mipLevel)
{
    m_dirty = true;

    TextureInfo info{};
    info.m_texture = texture;
    info.m_mipLevel = mipLevel;

    m_pendingTextures.emplace_back(slot, info);
    s_storage.AddTexData(m_shader, texture, slot);
}

void Material::Sync()
{
    if (!m_dirty)
    {
        return;
    }
    m_dirty = false;

    if (!m_gpuMaterial)
    {
        auto& rs = Instance().Service<RenderService>();
        m_gpuMaterial = rs.CreateGPUMaterial(m_shader);
    }

    for (auto&& [slot, buffer] : m_pendingBuffers)
    {
        m_gpuMaterial->SetBuffer(buffer.m_gpuBuffer, slot, buffer.m_stage, buffer.m_offset);
        m_buffers[slot] = std::move(buffer);
    }

    for (auto&& [slot, texture] : m_pendingTextures)
    {
        m_gpuMaterial->SetTexture(texture.m_texture, slot, texture.m_mipLevel);
        m_textures[slot] = std::move(texture);
    }

    m_gpuMaterial->Sync();

    m_pendingBuffers.clear();
    m_pendingTextures.clear();
}

} // engine::render