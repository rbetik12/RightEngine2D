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
}

void Material::SetTexture(const std::shared_ptr<rhi::Texture>& texture, int slot)
{
    m_dirty = true;

    TextureInfo info{};
    info.m_texture = texture;

    m_pendingTextures.emplace_back(slot, info);
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
        m_gpuMaterial->SetTexture(texture.m_texture, slot);
        m_textures[slot] = std::move(texture);
    }

    m_gpuMaterial->Sync();

    m_pendingBuffers.clear();
    m_pendingTextures.clear();
}

} // engine::render