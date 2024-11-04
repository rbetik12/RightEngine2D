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

class ENGINE_API MaterialDataStorage
{
public:
    struct TextureData
    {
        std::weak_ptr<rhi::Texture> m_texture;
        uint8_t                     m_slot;

        bool operator<(const TextureData& other) const
        {
            return m_slot < other.m_slot;
        }
    };

    using TexDataSet = eastl::set<TextureData>;

    TexDataSet TexData(const std::shared_ptr<rhi::Shader>& shader) const
    {
        if (const auto it = m_texData.find(shader->Descriptor().m_path); it != m_texData.end())
        {
            return it->second;
        }

        return {};
    }

    void AddTexData(const std::shared_ptr<rhi::Shader>& shader, const std::shared_ptr<rhi::Texture>& texture, uint8_t slot)
    {
        m_texData[shader->Descriptor().m_path].insert({ texture, slot });
    }

    void ClearTexData(const std::shared_ptr<rhi::Shader>& shader)
    {
        m_texData.erase(shader->Descriptor().m_path);
    }

private:
    eastl::unordered_map<io::fs::path, TexDataSet> m_texData;
};

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
        info.m_gpuBuffer->CopyToBuffer(info.m_cpuBuffer.get_raw_ptr(), info.m_cpuBuffer.get_type().get_sizeof());
    }

    template <typename T>
    inline T* Buffer(int slot)
    {
        ENGINE_ASSERT(registration::helpers::typeRegistered<T>());

        if (slot > m_buffers.size())
        {
            return nullptr;
        }

        auto& info = m_buffers[slot];

        ENGINE_ASSERT(info.m_cpuBuffer.is_valid());
        if (info.m_cpuBuffer.get_type().get_name() != rttr::type::get<T>().get_name())
        {
            return nullptr;
        }

        return &info.m_cpuBuffer.get_value_safe<T>();
    }

    void SetBuffer(rttr::type type, int slot, rhi::ShaderStage stage, std::string_view name = "", int offset = 0);

    void SetTexture(const std::shared_ptr<rhi::Texture>& texture, uint8_t slot, uint8_t mipLevel = 0);

    void Sync();

private:
    void UpdateBuffer(int slot);

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
        uint8_t                       m_mipLevel = 0;
    };

    eastl::vector<BufferInfo>                        m_buffers;
    eastl::vector<TextureInfo>                       m_textures;
    eastl::vector<eastl::pair<uint8_t, BufferInfo>>  m_pendingBuffers;
    eastl::vector<eastl::pair<uint8_t, TextureInfo>> m_pendingTextures;
    std::shared_ptr<rhi::Shader>                     m_shader;
    std::shared_ptr<rhi::GPUMaterial>                m_gpuMaterial;
    bool                                             m_dirty;

    inline static MaterialDataStorage                s_storage;
};

} // engine::render