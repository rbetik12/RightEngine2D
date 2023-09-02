#pragma once

#include "Types.hpp"
#include "Shader.hpp"
#include "RendererAPI.hpp"

namespace RightEngine
{
    class RendererCommand
    {
    public:
        static void Init(GPU_API api);
        static void BeginFrame(const std::shared_ptr<CommandBuffer>& cmd,
                               const std::shared_ptr<GraphicsPipeline>& pipeline);
        static void EndFrame(const std::shared_ptr<CommandBuffer>& cmd,
                             const std::shared_ptr<GraphicsPipeline>& pipeline);

        static void SetClearColor(const glm::vec4& color);

        static void Clear(uint32_t clearBits);

        static void SetViewport(const Viewport& viewport);
        static Viewport GetViewport();

        static void Draw(const std::shared_ptr<CommandBuffer>& cmd,
                         const std::shared_ptr<Buffer>& buffer,
                         uint32_t vertexCount,
                         uint32_t instanceCount = 1);
        static void DrawIndexed(const std::shared_ptr<CommandBuffer>& cmd,
                                const std::shared_ptr<Buffer>& vertexBuffer,
                                const std::shared_ptr<Buffer>& indexBuffer,
                                uint32_t indexCount,
                                uint32_t instanceCount = 1);

        static void EncodeState(const std::shared_ptr<CommandBuffer>& cmd,
                                const std::shared_ptr<GraphicsPipeline>& pipeline,
                                const std::shared_ptr<RendererState>& state);

        static std::shared_ptr<RendererState> CreateRendererState();

        static void Configure(const RendererSettings& settings);

        static const std::shared_ptr<RenderingContext>& GetContext();

    private:
        static std::shared_ptr<RendererAPI> rendererAPI;
    };
}
