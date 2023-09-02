#include "RendererCommand.hpp"

using namespace RightEngine;

std::shared_ptr<RendererAPI> RendererCommand::rendererAPI = nullptr;

void RendererCommand::Init(GPU_API api)
{
    rendererAPI = RendererAPI::Create(api);
    rendererAPI->Init();
}

void RendererCommand::SetClearColor(const glm::vec4& color)
{
    rendererAPI->SetClearColor(color);
}

void RendererCommand::Clear(uint32_t clearBits)
{
    // TODO: Use API agnostic bits
    rendererAPI->Clear(clearBits);
}

void RendererCommand::SetViewport(const Viewport& viewport)
{
    rendererAPI->SetViewport(viewport);
}

Viewport RendererCommand::GetViewport()
{
    return rendererAPI->GetViewport();
}

void RendererCommand::Configure(const RendererSettings& settings)
{
    rendererAPI->Configure(settings);
}

void RendererCommand::Draw(const std::shared_ptr<CommandBuffer>& cmd,
                           const std::shared_ptr<Buffer>& buffer,
                           uint32_t vertexCount,
                           uint32_t instanceCount)
{
    R_CORE_ASSERT(buffer->GetDescriptor().type == BufferType::VERTEX
                  && buffer->GetDescriptor().size > 0
                  && vertexCount > 0
                  && instanceCount > 0, "");
    rendererAPI->Draw(cmd, buffer, vertexCount, instanceCount);
}

void RendererCommand::DrawIndexed(const std::shared_ptr<CommandBuffer>& cmd,
                                  const std::shared_ptr<Buffer>& vertexBuffer,
                                  const std::shared_ptr<Buffer>& indexBuffer,
                                  uint32_t indexCount,
                                  uint32_t instanceCount)
{
    R_CORE_ASSERT(vertexBuffer->GetDescriptor().type == BufferType::VERTEX
                  && indexBuffer->GetDescriptor().type == BufferType::INDEX
                  && vertexBuffer->GetDescriptor().size > 0
                  && indexBuffer->GetDescriptor().size > 0
                  && indexCount > 0
                  && instanceCount > 0, "");
    rendererAPI->Draw(cmd, vertexBuffer, indexBuffer, indexCount, instanceCount);
}

void RendererCommand::EncodeState(const std::shared_ptr<CommandBuffer>& cmd,
                 const std::shared_ptr<GraphicsPipeline>& pipeline,
                 const std::shared_ptr<RendererState>& state)
{
    rendererAPI->EncodeState(cmd, pipeline, state);
}

void RendererCommand::BeginFrame(const std::shared_ptr<CommandBuffer>& cmd,
                                 const std::shared_ptr<GraphicsPipeline>& pipeline)
{
    rendererAPI->BeginFrame(cmd, pipeline);
}

void RendererCommand::EndFrame(const std::shared_ptr<CommandBuffer>& cmd,
                               const std::shared_ptr<GraphicsPipeline>& pipeline)
{
    rendererAPI->EndFrame(cmd, pipeline);
}

std::shared_ptr<RendererState> RendererCommand::CreateRendererState()
{
    return rendererAPI->CreateRendererState();
}

const std::shared_ptr<RenderingContext>& RendererCommand::GetContext()
{
    return rendererAPI->GetContext();
}
