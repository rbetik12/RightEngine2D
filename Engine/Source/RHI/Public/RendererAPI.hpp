#pragma once

#include "Types.hpp"
#include "RenderingContext.hpp"
#include "Buffer.hpp"
#include "RendererState.hpp"
#include <glm/glm.hpp>
#include <memory>

namespace RightEngine
{
    struct RendererSettings;

    struct Viewport
    {
        int32_t x;
        int32_t y;
        int32_t width;
        int32_t height;
    };

    class RendererAPI
    {
    public:
        virtual ~RendererAPI() = default;

        virtual void Init() = 0;
        virtual void BeginFrame(const std::shared_ptr<CommandBuffer>& cmd,
                                const std::shared_ptr<GraphicsPipeline>& pipeline) = 0;
        virtual void EndFrame(const std::shared_ptr<CommandBuffer>& cmd,
                              const std::shared_ptr<GraphicsPipeline>& pipeline) = 0;
        virtual void Configure(const RendererSettings& settings) = 0;
        virtual void SetClearColor(const glm::vec4& color) = 0;
        virtual void Clear(uint32_t clearBits) = 0;
        virtual void SetViewport(const Viewport& viewport) = 0;
        virtual Viewport GetViewport() = 0;
        virtual void Draw(const std::shared_ptr<CommandBuffer>& cmd,
                          const std::shared_ptr<Buffer>& buffer,
                          uint32_t vertexCount,
                          uint32_t instanceCount = 1) = 0;
        virtual void Draw(const std::shared_ptr<CommandBuffer>& cmd,
                          const std::shared_ptr<Buffer>& vertexBuffer,
                          const std::shared_ptr<Buffer>& indexBuffer,
                          uint32_t indexCount,
                          uint32_t instanceCount = 1) = 0;

        virtual void EncodeState(const std::shared_ptr<CommandBuffer>& cmd,
                                 const std::shared_ptr<GraphicsPipeline>& pipeline,
                                 const std::shared_ptr<RendererState>& state) = 0;

        virtual std::shared_ptr<RendererState> CreateRendererState() = 0;

        virtual const std::shared_ptr<RenderingContext>& GetContext() const = 0;

        static std::shared_ptr<RendererAPI> Create(GPU_API GpuApi);
        static GPU_API GetAPI();

    protected:
        static GPU_API api;
        bool isInitialized{ false };
    };
}
