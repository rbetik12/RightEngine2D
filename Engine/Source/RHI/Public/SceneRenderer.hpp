#pragma once

#include "Scene.hpp"
#include "MeshLoader.hpp"
#include "UniformBufferSet.hpp"
#include "Renderer.hpp"

namespace RightEngine
{
    struct SceneRendererSettings
    {
        float gamma{ 2.2f };
    };

    enum class PassType
    {
        SHADOW,
        PBR,
        SKYBOX,
        POSTPROCESS,
        UI,
        PRESENT
    };

    struct LightData
    {
        glm::vec4 color;
        glm::vec4 position;
        glm::vec4 rotation;
        float intensity;
        int type;
        float radiusInner;
        float radiusOuter;
        glm::mat4 lightSpace;
    };

    struct CameraData
    {
        glm::vec3 position;
        glm::mat4 view;
        glm::mat4 projection;
    };

    struct PassInfo
    {
        std::string m_name;
        double m_time;
    };

    class SceneRenderer
    {
    public:
        void Init();

        const std::shared_ptr<Scene>& GetScene() const
        { return scene; }

        void SetScene(const std::shared_ptr<Scene>& aScene)
        { scene = aScene; }

        void SubmitMeshNode(const std::shared_ptr<MeshNode>& meshNode, const std::shared_ptr<Material>& material, const glm::mat4& transform);
        void SubmitMesh(const std::shared_ptr<Mesh>& mesh, const std::shared_ptr<Material>& material, const glm::mat4& transform);

        void BeginScene(const CameraData& cameraData,
                        const std::shared_ptr<EnvironmentContext>& environment,
                        const std::vector<LightData>& lights,
                        const SceneRendererSettings& rendererSettings = {});
        void EndScene();

        void Resize(int x, int y);

        void SetUIPassCallback(std::function<void(const std::shared_ptr<CommandBuffer>&)>&& callback)
        { uiPassCallback = callback; }

        uint32_t Pick(const std::shared_ptr<Scene>& scene, const glm::vec2& pos);

        const std::shared_ptr<GraphicsPipeline>& GetPass(PassType type) const;
        const std::shared_ptr<Texture>& GetFinalImage() const;

        const std::vector<PassInfo> GetPassInfo() const { return m_passInfo; }

    private:
        void CreateShaders();
        void CreatePasses();
        void CreateOffscreenPasses();
        void CreateOnscreenPasses();
        void CreateBuffers();

        // Passes
        void ShadowPass();
        void PBRPass();
        void SkyboxPass();
        void PostprocessPass();
        void UIPass();
        void Present();
        void Clear();

        // Renderer data
        std::shared_ptr<Scene> scene;
        std::shared_ptr<Sampler> defaultSampler;
        bool isInitialized{ false };
        glm::ivec2 viewport{ 1280, 720 };
        glm::ivec2 windowSize{ 1280, 720 };
        std::shared_ptr<UniformBufferSet> uniformBufferSet;
        Renderer renderer;
        std::shared_ptr<Buffer> skyboxVertexBuffer;
        std::shared_ptr<Buffer> fullscreenQuadVertexBuffer;
        std::function<void(const std::shared_ptr<CommandBuffer>&)> uiPassCallback;

        //Pipelines
        std::shared_ptr<GraphicsPipeline> pbrPipeline;
        std::shared_ptr<GraphicsPipeline> skyboxPipeline;
        std::shared_ptr<GraphicsPipeline> postprocessPipeline;
        std::shared_ptr<GraphicsPipeline> uiPipeline;
        std::shared_ptr<GraphicsPipeline> presentPipeline;
        std::shared_ptr<GraphicsPipeline> m_pickingPipeline;
        std::shared_ptr<GraphicsPipeline> m_shadowPipeline;

        // TODO: Move shaders to ShaderLibrary
        std::shared_ptr<Shader> pbrShader;
        std::shared_ptr<Shader> skyboxShader;
        std::shared_ptr<Shader> postprocessShader;
        std::shared_ptr<Shader> m_pickingShader;
        std::shared_ptr<Shader> m_shadowShader;

        struct DrawCommand
        {
            std::shared_ptr<Mesh> mesh;
            std::shared_ptr<Material> material;
            glm::mat4 transform;
        };

        struct UBCameraData
        {
            glm::mat4 viewProjection;
            glm::vec4 position;
        } cameraDataUB;

        struct UBLightData
        {
            LightData light[30];
            int lightsAmount;
            glm::vec3 dummy;
        } lightDataUB;

        struct UBTransformData
        {
            glm::mat4 transform;
        } transformDataUB;

        struct UBColorId
        {
            glm::vec4 color;
        } colorIdUB;

        // Per frame data
        std::vector<DrawCommand> m_drawList;
        EnvironmentContext sceneEnvironment;
        CameraData camera;
        std::vector<PassInfo> m_passInfo;
    };
}