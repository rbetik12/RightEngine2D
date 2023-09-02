#pragma once

#include "Texture.hpp"
#include "Buffer.hpp"
#include "ShaderProgramDescriptor.hpp"
#include "Utils.hpp"
#include <unordered_map>

namespace RightEngine
{
    struct BufferOffset
    {
        int offset;
        int stride;
    };

    class RendererState
    {
    public:
        const std::shared_ptr<Texture>& GetTexture(int slot);
        void SetTexture(const std::shared_ptr<Texture>& texture, int slot);

        const std::shared_ptr<Buffer>& GetFragmentBuffer(int slot);
        void SetFragmentBuffer(const std::shared_ptr<Buffer>& buffer, int slot, int offset = 0, int stride = 0);

        const std::shared_ptr<Buffer>& GetVertexBuffer(int slot);
        void SetVertexBuffer(const std::shared_ptr<Buffer>& buffer, int slot, int offset = 0, int stride = 0);

        virtual void OnUpdate(const std::shared_ptr<GraphicsPipeline>& pipeline) = 0;

        bool IsSyncNeeded() const
        { return isSyncNeeded; }

        virtual ~RendererState() = default;

    protected:
        std::unordered_map<int, std::shared_ptr<Texture>> textures;
        std::unordered_map<BufferRef, std::shared_ptr<Buffer>, BufferRefHash> buffers;
        std::unordered_map<BufferRef, BufferOffset, BufferRefHash> offsets;

        bool isSyncNeeded{ false };
        std::vector<std::pair<BufferRef, std::weak_ptr<Buffer>>> buffersToSync;
        std::vector<std::pair<int, std::weak_ptr<Texture>>> texturesToSync;

    private:
        void SetBuffer(const std::shared_ptr<Buffer>& buffer, int slot, ShaderStage stage, int offset = 0, int stride = 0);
    };
}
