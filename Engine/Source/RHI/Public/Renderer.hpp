#pragma once

#include "Window.hpp"
#include "Components.hpp"
#include "Config.hpp"
#include "Shader.hpp"
#include "Scene.hpp"
#include "Types.hpp"
#include "RendererState.hpp"
#include "MeshLoader.hpp"
#include <glm/matrix.hpp>

namespace RightEngine
{
    struct SceneData
    {
        glm::mat4 viewProjectionMatrix{ 1.0f };
    };

    enum class DepthTestMode
    {
        NEVER,
        LESS,
        EQUAL,
        LEQUAL,
        GREATER,
        NOTEQUAL,
        GEQUAL,
        ALWAYS
    };

    struct RendererSettings
    {
        bool hasDepthTest{ true };
        DepthTestMode depthTestMode{ DepthTestMode::LESS };
    };

    class Renderer
    {
    public:
        Renderer();

        void BeginFrame();
        void EndFrame();

        void Draw(const std::shared_ptr<Buffer>& vertexBuffer, const std::shared_ptr<Buffer>& indexBuffer = nullptr);
        void Draw(const MeshComponent& meshComponent);
        void Draw(const std::shared_ptr<MeshNode>& meshNode);
        void Draw(const std::shared_ptr<Mesh>& mesh);
        void EncodeState(const std::shared_ptr<RendererState>& state);

        void SetPipeline(const std::shared_ptr<GraphicsPipeline>& aPipeline);
        const std::shared_ptr<GraphicsPipeline>& GetActivePipeline() const
        { return pipeline; }

        void Configure();

        // TODO: Must be removed
        const std::shared_ptr<CommandBuffer>& GetCmd() const
        { return commandBuffer; }

        RendererSettings& GetSettings();

        static GPU_API GetAPI();

    private:
        SceneData sceneData;
        RendererSettings settings;
        std::shared_ptr<CommandBuffer> commandBuffer;
        std::shared_ptr<GraphicsPipeline> pipeline;
    };
}