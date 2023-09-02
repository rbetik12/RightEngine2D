#include "SceneRenderer.hpp"
#include "GraphicsPipeline.hpp"
#include "RendererState.hpp"
#include "RendererCommand.hpp"
#include "AssetManager.hpp"
#include "Application.hpp"
#include "Timer.hpp"
#include "RHIHelpers.hpp"
#include <Engine/Service/ThreadService.hpp>
#include <stb_image_write.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

using namespace RightEngine;

namespace
{
    float skyboxVertices[] = {
            // back face
            -1.0f, -1.0f, -1.0f,   // bottom-left
            1.0f, 1.0f, -1.0f,   // top-right
            1.0f, -1.0f, -1.0f,   // bottom-right
            1.0f, 1.0f, -1.0f,   // top-right
            -1.0f, -1.0f, -1.0f,   // bottom-left
            -1.0f, 1.0f, -1.0f,   // top-left
            // front face
            -1.0f, -1.0f, 1.0f,   // bottom-left
            1.0f, -1.0f, 1.0f,   // bottom-right
            1.0f, 1.0f, 1.0f,   // top-right
            1.0f, 1.0f, 1.0f,   // top-right
            -1.0f, 1.0f, 1.0f,   // top-left
            -1.0f, -1.0f, 1.0f,   // bottom-left
            // left face
            -1.0f, 1.0f, 1.0f,  // top-right
            -1.0f, 1.0f, -1.0f,  // top-left
            -1.0f, -1.0f, -1.0f,  // bottom-left
            -1.0f, -1.0f, -1.0f,  // bottom-left
            -1.0f, -1.0f, 1.0f,  // bottom-right
            -1.0f, 1.0f, 1.0f,  // top-right
            // right face
            1.0f, 1.0f, 1.0f,   // top-left
            1.0f, -1.0f, -1.0f,   // bottom-right
            1.0f, 1.0f, -1.0f,   // top-right
            1.0f, -1.0f, -1.0f,  // bottom-right
            1.0f, 1.0f, 1.0f,   // top-left
            1.0f, -1.0f, 1.0f,   // bottom-left
            // bottom face
            -1.0f, -1.0f, -1.0f,   // top-right
            1.0f, -1.0f, -1.0f,   // top-left
            1.0f, -1.0f, 1.0f,   // bottom-left
            1.0f, -1.0f, 1.0f,   // bottom-left
            -1.0f, -1.0f, 1.0f,   // bottom-right
            -1.0f, -1.0f, -1.0f,   // top-right
            // top face
            -1.0f, 1.0f, -1.0f,  // top-left
            1.0f, 1.0f, 1.0f,   // bottom-right
            1.0f, 1.0f, -1.0f,   // top-right
            1.0f, 1.0f, 1.0f,   // bottom-right
            -1.0f, 1.0f, -1.0f,   // top-left
            -1.0f, 1.0f, 1.0f,    // bottom-left
    };

    float quadVertices[] = {
            -1.0, 1.0, 0.0, 1.0,
            1.0, 1.0, 1.0, 1.0,
            1.0, -1.0, 1.0, 0.0,
            1.0, -1.0, 1.0, 0.0,
            -1.0, -1.0, 0.0, 0.0,
            -1.0, 1.0, 0.0, 1.0
    };

    std::shared_ptr<Texture> GetTexture(const AssetHandle& handle)
    {
        return AssetManager::Get().GetAsset<Texture>(handle);
    }

    glm::vec4 IdToColor(uint32_t id)
    {
        uint32_t r = (id & 0x000000FF) >> 0;
        uint32_t g = (id & 0x0000FF00) >> 8;
        uint32_t b = (id & 0x00FF0000) >> 16;
        return glm::vec4(r, g, b, 255.0f);
    }

    struct Pixel
    {
        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t a;

        bool Empty() const
        {
            return r == 0 && g == 0 && b == 0;
        }
    };

    uint32_t ColorToId(Pixel pixel)
    {
        uint32_t id = pixel.b + pixel.g * 256 + pixel.r * 256 * 256;
        return id;
    }

    void SaveTexture(const std::shared_ptr<Texture>& texture)
    {
        auto buffer = texture->Data();
        auto& desc = texture->GetSpecification();
        auto ptr = buffer->Map();

        struct Pixel
        {
            uint8_t r;
            uint8_t g;
            uint8_t b;
            uint8_t a;

            bool Empty() const
            {
                return r == 0 && g == 0 && b == 0;
            }
        };
        std::vector<Pixel> pickData(buffer->GetDescriptor().size / sizeof(Pixel));
        memcpy(pickData.data(), ptr, buffer->GetDescriptor().size);
        buffer->UnMap();
        stbi_write_bmp("image.bmp", desc.width, desc.height, desc.componentAmount, pickData.data());
    }

    constexpr const int C_SHADOWMAP_WIDTH = 1024;
    constexpr const int C_SHADOWMAP_HEIGHT = 1024;
}

void SceneRenderer::Init()
{
    R_CORE_ASSERT(!isInitialized, "");
    isInitialized = true;

    defaultSampler = Device::Get()->CreateSampler({});
    const auto& window = Application::Get().GetWindow();
    windowSize = { window->GetWidth(), window->GetHeight() };

    CreateShaders();
    CreatePasses();
    CreateBuffers();
}

void SceneRenderer::CreateShaders()
{
    auto& ts = Instance().Service<engine::ThreadService>();

    tf::Taskflow taskflow;

    //PBR
    taskflow.emplace([=]()
	    {
		    ShaderProgramDescriptor shaderProgramDescriptor;
		    ShaderDescriptor vertexShader;
		    vertexShader.path = "/Engine/Shaders/pbr.vert";
		    vertexShader.type = ShaderType::VERTEX;
		    ShaderDescriptor fragmentShader;
		    fragmentShader.path = "/Engine/Shaders/pbr.frag";
		    fragmentShader.type = ShaderType::FRAGMENT;
		    shaderProgramDescriptor.shaders = {vertexShader, fragmentShader};
		    VertexBufferLayout layout;
		    layout.Push<glm::vec3>();
		    layout.Push<glm::vec3>();
		    layout.Push<glm::vec2>();
		    layout.Push<glm::vec3>();
		    layout.Push<glm::vec3>();
		    shaderProgramDescriptor.layout = layout;
		    shaderProgramDescriptor.reflection.textures = {3, 4, 5, 6, 7, 8, 9, 10, 13};
		    shaderProgramDescriptor.reflection.buffers[{0, ShaderType::VERTEX}] = BufferType::UNIFORM;
		    shaderProgramDescriptor.reflection.buffers[{1, ShaderType::VERTEX}] = BufferType::UNIFORM;
		    shaderProgramDescriptor.reflection.buffers[{2, ShaderType::FRAGMENT}] = BufferType::UNIFORM;
		    shaderProgramDescriptor.reflection.buffers[{11, ShaderType::FRAGMENT}] = BufferType::UNIFORM;
		    shaderProgramDescriptor.reflection.buffers[{12, ShaderType::FRAGMENT}] = BufferType::UNIFORM;
		    pbrShader = Device::Get()->CreateShader(shaderProgramDescriptor);
	    }
    );

    //Skybox
    taskflow.emplace([=]()
        {
            ShaderProgramDescriptor skyboxShaderDesc{};
		    ShaderDescriptor vertexShader;
		    vertexShader.path = "/Engine/Shaders/skybox.vert";
		    vertexShader.type = ShaderType::VERTEX;
		    ShaderDescriptor fragmentShader;
		    fragmentShader.path = "/Engine/Shaders/skybox.frag";
		    fragmentShader.type = ShaderType::FRAGMENT;
		    skyboxShaderDesc.shaders = { vertexShader, fragmentShader };
		    VertexBufferLayout skyboxLayout;
		    skyboxLayout.Push<glm::vec3>();
		    skyboxShaderDesc.layout = skyboxLayout;
		    skyboxShaderDesc.reflection.textures = { 3 };
		    skyboxShaderDesc.reflection.buffers[{ 1, ShaderType::VERTEX }] = BufferType::UNIFORM;
		    skyboxShader = Device::Get()->CreateShader(skyboxShaderDesc);
        }
	);

    //Postprocess
    taskflow.emplace([=]()
        {
            ShaderProgramDescriptor postprocessShaderDesc{};
		    ShaderDescriptor vertexShader;
		    vertexShader.path = "/Engine/Shaders/postprocess.vert";
		    vertexShader.type = ShaderType::VERTEX;
		    ShaderDescriptor fragmentShader;
		    fragmentShader.path = "/Engine/Shaders/postprocess.frag";
		    fragmentShader.type = ShaderType::FRAGMENT;
		    postprocessShaderDesc.shaders = { vertexShader, fragmentShader };
		    VertexBufferLayout postprocessLayout;
		    postprocessLayout.Push<glm::vec2>();
		    postprocessLayout.Push<glm::vec2>();
		    postprocessShaderDesc.layout = postprocessLayout;
		    postprocessShaderDesc.reflection.textures = { 3 };
		    postprocessShaderDesc.reflection.buffers[{ 12, ShaderType::FRAGMENT }] = BufferType::UNIFORM;
		    postprocessShader = Device::Get()->CreateShader(postprocessShaderDesc);
        }
	);

    //Picking
    taskflow.emplace([=]()
        {
            ShaderProgramDescriptor shaderDesc{};
		    ShaderDescriptor vertexShader;
		    vertexShader.path = "/Engine/Shaders/Utils/picking.vert";
		    vertexShader.type = ShaderType::VERTEX;
		    ShaderDescriptor fragmentShader;
		    fragmentShader.path = "/Engine/Shaders/Utils/picking.frag";
		    fragmentShader.type = ShaderType::FRAGMENT;
            shaderDesc.shaders = { vertexShader, fragmentShader };
		    VertexBufferLayout layout;
            layout.Push<glm::vec3>();
            layout.Push<glm::vec3>();
            layout.Push<glm::vec2>();
            layout.Push<glm::vec3>();
            layout.Push<glm::vec3>();
            shaderDesc.layout = layout;
            shaderDesc.reflection.buffers[{0, ShaderType::VERTEX}] = BufferType::UNIFORM;
            shaderDesc.reflection.buffers[{1, ShaderType::VERTEX}] = BufferType::UNIFORM;
            shaderDesc.reflection.buffers[{ 13, ShaderType::FRAGMENT }] = BufferType::UNIFORM;
		    m_pickingShader = Device::Get()->CreateShader(shaderDesc);
        }
	);

    //Shadow
    taskflow.emplace([=]()
        {
            ShaderProgramDescriptor desc{};
			ShaderDescriptor vertex = helpers::CreateShaderDescriptor("/Engine/Shaders/shadow.vert", ShaderType::VERTEX);
            ShaderDescriptor fragment = helpers::CreateShaderDescriptor("/Engine/Shaders/shadow.frag", ShaderType::FRAGMENT);
            desc.shaders = { vertex, fragment };
            VertexBufferLayout layout;
            layout.Push<glm::vec3>();
            layout.Push<glm::vec3>();
            layout.Push<glm::vec2>();
            layout.Push<glm::vec3>();
            layout.Push<glm::vec3>();
            desc.layout = layout;
            desc.reflection.buffers[{0, ShaderType::VERTEX}] = BufferType::UNIFORM;
            desc.reflection.buffers[{ C_CONSTANT_BUFFER_SLOT, ShaderType::VERTEX}] = BufferType::CONSTANT;
            m_shadowShader = Device::Get()->CreateShader(desc);
        });

    ts.AddBackgroundTaskflow(std::move(taskflow)).wait();
}

void SceneRenderer::CreatePasses()
{
    CreateOffscreenPasses();
    CreateOnscreenPasses();
}

void SceneRenderer::CreateOffscreenPasses()
{
    //PBR
    {
        TextureDescriptor colorAttachmentDesc{};
        colorAttachmentDesc.format = Format::BGRA8_UNORM;
        colorAttachmentDesc.type = TextureType::TEXTURE_2D;
        colorAttachmentDesc.width = viewport.x;
        colorAttachmentDesc.height = viewport.y;
        const auto colorAttachment = Device::Get()->CreateTexture(colorAttachmentDesc, {});
        colorAttachment->SetSampler(defaultSampler);
        TextureDescriptor depthAttachmentDesc{};
        depthAttachmentDesc.format = Format::D32_SFLOAT_S8_UINT;
        depthAttachmentDesc.type = TextureType::TEXTURE_2D;
        depthAttachmentDesc.width = viewport.x;
        depthAttachmentDesc.height = viewport.y;
        const auto depthAttachment = Device::Get()->CreateTexture(depthAttachmentDesc, {});

        GraphicsPipelineDescriptor pipelineDescriptor{};
        pipelineDescriptor.shader = pbrShader;

        RenderPassDescriptor renderPassDescriptor{};
        renderPassDescriptor.extent = viewport;
        renderPassDescriptor.offscreen = true;
        renderPassDescriptor.name = "PBR_main";
        AttachmentDescriptor depth{};
        depth.loadOperation = AttachmentLoadOperation::CLEAR;
        depth.texture = depthAttachment;
        AttachmentDescriptor color{};
        color.texture = colorAttachment;
        color.loadOperation = AttachmentLoadOperation::CLEAR;
        color.storeOperation = AttachmentStoreOperation::STORE;
        renderPassDescriptor.colorAttachments = { color };
        renderPassDescriptor.depthStencilAttachment = { depth };
        pbrPipeline = Device::Get()->CreateGraphicsPipeline(pipelineDescriptor, renderPassDescriptor);
    }

    //Skybox
    {
        const auto& pbrPassDescriptor = pbrPipeline->GetRenderPassDescriptor();
        R_CORE_ASSERT(pbrPassDescriptor.colorAttachments.front().texture
                      && pbrPassDescriptor.depthStencilAttachment.texture, "");

        GraphicsPipelineDescriptor pipelineDescriptor;
        pipelineDescriptor.shader = skyboxShader;
        pipelineDescriptor.depthCompareOp = CompareOp::LESS_OR_EQUAL;
        pipelineDescriptor.cullMode = CullMode::FRONT;

        RenderPassDescriptor renderPassDescriptor{};
        renderPassDescriptor.name = "Skybox";
        renderPassDescriptor.extent = viewport;
        renderPassDescriptor.offscreen = true;

        AttachmentDescriptor depth{};
        depth.loadOperation = AttachmentLoadOperation::LOAD;
        depth.texture = pbrPassDescriptor.depthStencilAttachment.texture;

        AttachmentDescriptor color{};
        color.loadOperation = AttachmentLoadOperation::LOAD;
        color.storeOperation = AttachmentStoreOperation::STORE;
        color.texture = pbrPassDescriptor.colorAttachments.front().texture;

        renderPassDescriptor.colorAttachments = { color };
        renderPassDescriptor.depthStencilAttachment = { depth };
        skyboxPipeline = Device::Get()->CreateGraphicsPipeline(pipelineDescriptor, renderPassDescriptor);
    }

    // Postprocess
    {
        GraphicsPipelineDescriptor pipelineDescriptor{};
        pipelineDescriptor.shader = postprocessShader;

        RenderPassDescriptor renderPassDescriptor{};
        renderPassDescriptor.name = "Postprocess";
        renderPassDescriptor.extent = viewport;
        renderPassDescriptor.offscreen = true;

        TextureDescriptor colorAttachmentDesc{};
        colorAttachmentDesc.format = Format::BGRA8_UNORM;
        colorAttachmentDesc.type = TextureType::TEXTURE_2D;
        colorAttachmentDesc.width = viewport.x;
        colorAttachmentDesc.height = viewport.y;
        const auto colorAttachment = Device::Get()->CreateTexture(colorAttachmentDesc, {});
        colorAttachment->SetSampler(defaultSampler);
        TextureDescriptor depthAttachmentDesc{};
        depthAttachmentDesc.format = Format::D32_SFLOAT_S8_UINT;
        depthAttachmentDesc.type = TextureType::TEXTURE_2D;
        depthAttachmentDesc.width = viewport.x;
        depthAttachmentDesc.height = viewport.y;
        const auto depthAttachment = Device::Get()->CreateTexture(depthAttachmentDesc, {});

        AttachmentDescriptor depth{};
        depth.loadOperation = AttachmentLoadOperation::CLEAR;
        depth.texture = depthAttachment;

        AttachmentDescriptor color{};
        color.loadOperation = AttachmentLoadOperation::LOAD;
        color.storeOperation = AttachmentStoreOperation::STORE;
        color.texture = colorAttachment;

        renderPassDescriptor.colorAttachments = { color };
        renderPassDescriptor.depthStencilAttachment = { depth };
        postprocessPipeline = Device::Get()->CreateGraphicsPipeline(pipelineDescriptor, renderPassDescriptor);
    }

    //Picking
    {
        TextureDescriptor colorAttachmentDesc{};
        colorAttachmentDesc.format = Format::BGRA8_UNORM;
        colorAttachmentDesc.type = TextureType::TEXTURE_2D;
        colorAttachmentDesc.width = viewport.x;
        colorAttachmentDesc.height = viewport.y;
        colorAttachmentDesc.componentAmount = 4;
        const auto colorAttachment = Device::Get()->CreateTexture(colorAttachmentDesc, {});
        colorAttachment->SetSampler(defaultSampler);
        TextureDescriptor depthAttachmentDesc{};
        depthAttachmentDesc.format = Format::D32_SFLOAT_S8_UINT;
        depthAttachmentDesc.type = TextureType::TEXTURE_2D;
        depthAttachmentDesc.width = viewport.x;
        depthAttachmentDesc.height = viewport.y;
        const auto depthAttachment = Device::Get()->CreateTexture(depthAttachmentDesc, {});

        GraphicsPipelineDescriptor pipelineDescriptor{};
        pipelineDescriptor.shader = m_pickingShader;

        RenderPassDescriptor renderPassDescriptor{};
        renderPassDescriptor.extent = viewport;
        renderPassDescriptor.offscreen = true;
        renderPassDescriptor.name = "Picking";
        AttachmentDescriptor depth{};
        depth.loadOperation = AttachmentLoadOperation::CLEAR;
        depth.texture = depthAttachment;
        AttachmentDescriptor color{};
        color.texture = colorAttachment;
        color.loadOperation = AttachmentLoadOperation::CLEAR;
        color.storeOperation = AttachmentStoreOperation::STORE;
        renderPassDescriptor.colorAttachments = { color };
        renderPassDescriptor.depthStencilAttachment = { depth };
        m_pickingPipeline = Device::Get()->CreateGraphicsPipeline(pipelineDescriptor, renderPassDescriptor);
    }

    //Shadow
    {
        TextureDescriptor depthDesc = helpers::CreateTextureDescriptor(C_SHADOWMAP_WIDTH, C_SHADOWMAP_HEIGHT, TextureType::TEXTURE_2D, Format::D32_SFLOAT);
        const auto depth = Device::Get()->CreateTexture(depthDesc, {});
        SamplerDescriptor samplerDesc;
        samplerDesc.addressModeU = AddressMode::ClampToEdge;
        samplerDesc.addressModeV = AddressMode::ClampToEdge;
        samplerDesc.addressModeW = AddressMode::ClampToEdge;
        samplerDesc.maxLod = 9.0f;
        depth->SetSampler(Device::Get()->CreateSampler(samplerDesc));

        GraphicsPipelineDescriptor pipelineDesc;
        pipelineDesc.shader = m_shadowShader;
        pipelineDesc.cullMode = CullMode::FRONT;

        AttachmentDescriptor depthAttachment = helpers::CreateAttachmentDescriptor(depth);
        RenderPassDescriptor renderPassDecs = helpers::CreateRenderPassDescriptor({ C_SHADOWMAP_WIDTH, C_SHADOWMAP_HEIGHT }, {}, depthAttachment);
        m_shadowPipeline = Device::Get()->CreateGraphicsPipeline(pipelineDesc, renderPassDecs);
    }
}

void SceneRenderer::CreateOnscreenPasses()
{
    // Present
    {
        TextureDescriptor presentAttachmentDesc{};
        presentAttachmentDesc.format = Format::BGRA8_UNORM;
        presentAttachmentDesc.type = TextureType::TEXTURE_2D;
        presentAttachmentDesc.width = windowSize.x;
        presentAttachmentDesc.height = windowSize.y;
        const auto presentAttachment = Device::Get()->CreateTexture(presentAttachmentDesc, {});
        presentAttachment->SetSampler(defaultSampler);

        GraphicsPipelineDescriptor presentPipelineDesc{};
        presentPipelineDesc.shader = nullptr;
        RenderPassDescriptor presentRenderPassDescriptor{};
        presentRenderPassDescriptor.extent = windowSize;
        presentRenderPassDescriptor.offscreen = false;
        AttachmentDescriptor presentColor{};
        presentColor.texture = presentAttachment;
        presentColor.loadOperation = AttachmentLoadOperation::LOAD;
        presentRenderPassDescriptor.colorAttachments = { presentColor };

        presentPipeline = Device::Get()->CreateGraphicsPipeline(presentPipelineDesc, presentRenderPassDescriptor);
    }

    // UI
    {
        const auto& presentPipelineAttachment = presentPipeline->GetRenderPassDescriptor().colorAttachments.front();

        AttachmentDescriptor uiAttachment{};
        uiAttachment.texture = presentPipelineAttachment.texture;
        uiAttachment.loadOperation = AttachmentLoadOperation::CLEAR;
        uiAttachment.storeOperation = AttachmentStoreOperation::STORE;

        RenderPassDescriptor uiRenderpass{};
        uiRenderpass.extent = windowSize;
        uiRenderpass.offscreen = true;
        uiRenderpass.colorAttachments.emplace_back(uiAttachment);

        GraphicsPipelineDescriptor uiPipelineDesc{};
        uiPipelineDesc.shader = nullptr;

        uiPipeline = Device::Get()->CreateGraphicsPipeline(uiPipelineDesc, uiRenderpass);
    }
}


void SceneRenderer::CreateBuffers()
{
    const size_t maxEntitiesAmount = 512;
    const size_t transformBufferSize = maxEntitiesAmount * sizeof(UBTransformData);
    const size_t materialDataBufferSize = maxEntitiesAmount * sizeof(MaterialData);
    m_drawList.reserve(maxEntitiesAmount);

    uniformBufferSet = std::make_shared<UniformBufferSet>(1);
    uniformBufferSet->Create(65536, 0);
    uniformBufferSet->Create(sizeof(UBCameraData), 1);
    uniformBufferSet->Create(65536, 2);
    uniformBufferSet->Create(sizeof(UBLightData), 11);
    uniformBufferSet->Create(sizeof(SceneRendererSettings), 12);
    uniformBufferSet->Create(65536, 13);

    {
        BufferDescriptor bufferDescriptor{};
        bufferDescriptor.size = sizeof(skyboxVertices);
        bufferDescriptor.type = BufferType::VERTEX;
        bufferDescriptor.memoryType = MemoryType::CPU_GPU;
        skyboxVertexBuffer = Device::Get()->CreateBuffer(bufferDescriptor, &skyboxVertices);
    }

    {
        BufferDescriptor bufferDescriptor{};
        bufferDescriptor.size = sizeof(quadVertices);
        bufferDescriptor.type = BufferType::VERTEX;
        bufferDescriptor.memoryType = MemoryType::CPU_GPU;
        fullscreenQuadVertexBuffer = Device::Get()->CreateBuffer(bufferDescriptor, &quadVertices);
    }

}

void SceneRenderer::SubmitMeshNode(const std::shared_ptr<MeshNode>& meshNode, const std::shared_ptr<Material>& material, const glm::mat4& transform)
{
    for (const auto& mesh: meshNode->meshes)
    {
        SubmitMesh(mesh, material, transform);
    }

    for (const auto& child: meshNode->children)
    {
        SubmitMeshNode(child, material, transform);
    }
}

void SceneRenderer::SubmitMesh(const std::shared_ptr<Mesh>& mesh, const std::shared_ptr<Material>& material, const glm::mat4& transform)
{
    DrawCommand dc;
    dc.mesh = mesh;
    dc.material = material;
    dc.transform = transform;

    m_drawList.emplace_back(dc);
}

void SceneRenderer::BeginScene(const CameraData& cameraData,
                               const std::shared_ptr<EnvironmentContext>& environment,
                               const std::vector<LightData>& lights,
                               const SceneRendererSettings& rendererSettings)
{
    R_CORE_ASSERT(lights.size() < 30, "");
    camera = cameraData;
    cameraDataUB.position = glm::vec4(camera.position, 1.0);
    auto projection = camera.projection;
    projection[1][1] *= -1;
    cameraDataUB.viewProjection = projection * camera.view;

    lightDataUB.lightsAmount = lights.size();
    memcpy(&lightDataUB.light, lights.data(), lights.size() * sizeof(LightData));
    sceneEnvironment = *environment;

    uniformBufferSet->Get(1)->SetData(&cameraDataUB, sizeof(cameraDataUB));
    uniformBufferSet->Get(11)->SetData(&lightDataUB, sizeof(lightDataUB));
    uniformBufferSet->Get(12)->SetData(&rendererSettings, sizeof(rendererSettings));
}

void SceneRenderer::EndScene()
{
	Timer timer;
    std::vector<PassInfo> passInfo;

    timer.Start();
    ShadowPass();
    passInfo.push_back({ "Shadow", timer.TimeInMilliseconds() });

    timer.Start();
    PBRPass();
    passInfo.push_back({ "PBR", timer.TimeInMilliseconds() });

    timer.Start();
    SkyboxPass();
    passInfo.push_back({ "Skybox", timer.TimeInMilliseconds() });

    timer.Start();
    PostprocessPass();
    passInfo.push_back({ "Postprocess", timer.TimeInMilliseconds() });

    timer.Start();
    UIPass();
    passInfo.push_back({ "UI", timer.TimeInMilliseconds() });

    timer.Start();
    Present();
    passInfo.push_back({ "Present", timer.TimeInMilliseconds() });
    Clear();
    m_passInfo = std::move(passInfo);
}

void SceneRenderer::ShadowPass()
{
    renderer.SetPipeline(m_shadowPipeline);
    renderer.BeginFrame();
    std::vector<std::shared_ptr<RendererState>> rendererStates(512);
    auto& transformBuffer = uniformBufferSet->Get(0);

    struct ConstantBuffer
    {
        glm::mat4 lightSpaceMatrix;
        glm::mat4 dummy1;
    } constantBuffer;

    std::vector<std::shared_ptr<Buffer>> lightBuffers;

    for (int lightIdx = 0; lightIdx < lightDataUB.lightsAmount; lightIdx++)
    {
        const auto& light = lightDataUB.light[lightIdx];
        if (light.type != 0)
        {
            continue;
        }

        constantBuffer.lightSpaceMatrix = light.lightSpace;
        constantBuffer.dummy1 = glm::mat4();

        BufferDescriptor desc{};
        desc.memoryType = MemoryType::CPU_GPU;
        desc.size = 128;
        desc.type = BufferType::CONSTANT;
        lightBuffers.push_back(Device::Get()->CreateBuffer(desc, &constantBuffer));

        uint32_t transformBufferOffset = 0;
        for (int i = 0; i < m_drawList.size(); i++)
        {
            auto& dc = m_drawList[i];
            const size_t transformDataSize = Device::Get()->GetAlignedGPUDataSize(sizeof(UBTransformData));
            transformBuffer->SetData(&dc.transform, transformDataSize, transformBufferOffset);

            auto& rs = rendererStates[i];
            rs = RendererCommand::CreateRendererState();
            rs->SetVertexBuffer(transformBuffer, 0, transformBufferOffset, sizeof(UBTransformData));
            rs->SetVertexBuffer(lightBuffers.back(), C_CONSTANT_BUFFER_SLOT, 0, 128);

            rs->OnUpdate(renderer.GetActivePipeline());
            renderer.EncodeState(rs);
            renderer.Draw(dc.mesh);

            transformBufferOffset += transformDataSize;
        }
    }

    renderer.EndFrame();
}

void SceneRenderer::PBRPass()
{
    renderer.SetPipeline(pbrPipeline);
    renderer.BeginFrame();
    std::vector<std::shared_ptr<RendererState>> rendererStates(512);

    auto& transformBuffer = uniformBufferSet->Get(0);
    auto& cameraBuffer = uniformBufferSet->Get(1);
    auto& materialBuffer = uniformBufferSet->Get(2);
    auto& lightBuffer = uniformBufferSet->Get(11);

    uint32_t transformBufferOffset = 0;
    uint32_t materialBufferOffset = 0;
    for (int i = 0; i < m_drawList.size(); i++)
    {
        auto& dc = m_drawList[i];
        const size_t transformDataSize = Device::Get()->GetAlignedGPUDataSize(sizeof(UBTransformData));
        const size_t materialDataSize = Device::Get()->GetAlignedGPUDataSize(sizeof(MaterialData));
        transformBuffer->SetData(&dc.transform, transformDataSize, transformBufferOffset);
        materialBuffer->SetData(&dc.material->materialData, materialDataSize, materialBufferOffset);

        auto& rs = rendererStates[i];
        rs = RendererCommand::CreateRendererState();
        rs->SetVertexBuffer(transformBuffer, 0, transformBufferOffset, sizeof(UBTransformData));
        rs->SetVertexBuffer(cameraBuffer, 1);
        rs->SetFragmentBuffer(materialBuffer, 2, materialBufferOffset, sizeof(MaterialData));
        rs->SetFragmentBuffer(lightBuffer, 11);

        rs->SetTexture(GetTexture(dc.material->textureData.albedo), 3);
        rs->SetTexture(GetTexture(dc.material->textureData.normal), 4);
        rs->SetTexture(GetTexture(dc.material->textureData.metallic), 5);
        rs->SetTexture(GetTexture(dc.material->textureData.roughness), 6);
        rs->SetTexture(GetTexture(dc.material->textureData.ao), 7);
        rs->SetTexture(sceneEnvironment.irradianceMap, 8);
        rs->SetTexture(sceneEnvironment.prefilterMap, 9);
        rs->SetTexture(sceneEnvironment.brdfLut, 10);
        rs->SetTexture(m_shadowPipeline->GetRenderPassDescriptor().depthStencilAttachment.texture, 13);

        rs->OnUpdate(renderer.GetActivePipeline());
        renderer.EncodeState(rs);
        renderer.Draw(dc.mesh);

        transformBufferOffset += transformDataSize;
        materialBufferOffset += materialDataSize;
    }
    renderer.EndFrame();
}

void SceneRenderer::SkyboxPass()
{
    renderer.SetPipeline(skyboxPipeline);
    renderer.BeginFrame();

    auto& cameraBuffer = uniformBufferSet->Get(1);

    UBCameraData skyboxCameraData{};
    auto projection = camera.projection;
    projection[1][1] *= -1;
    skyboxCameraData.viewProjection = projection * glm::mat4(glm::mat3(camera.view));
    cameraBuffer->SetData(&skyboxCameraData, sizeof(UBCameraData));

    auto rs = RendererCommand::CreateRendererState();
    rs->SetVertexBuffer(cameraBuffer, 1);
    rs->SetTexture(sceneEnvironment.envMap, 3);
    rs->OnUpdate(renderer.GetActivePipeline());

    renderer.EncodeState(rs);
    renderer.Draw(skyboxVertexBuffer);
    renderer.EndFrame();
}

void SceneRenderer::PostprocessPass()
{
    renderer.SetPipeline(postprocessPipeline);
    renderer.BeginFrame();

    auto& settingsBuffer = uniformBufferSet->Get(12);

    auto rs = RendererCommand::CreateRendererState();
    rs->SetTexture(pbrPipeline->GetRenderPassDescriptor().colorAttachments.front().texture, 3);
    rs->SetFragmentBuffer(settingsBuffer, 12);

    rs->OnUpdate(renderer.GetActivePipeline());
    renderer.EncodeState(rs);
    renderer.Draw(fullscreenQuadVertexBuffer);
    renderer.EndFrame();
}

void SceneRenderer::UIPass()
{
    renderer.SetPipeline(uiPipeline);
    renderer.BeginFrame();
    R_CORE_ASSERT(uiPassCallback, "");
    uiPassCallback(renderer.GetCmd());
    renderer.EndFrame();
}

void SceneRenderer::Present()
{
    renderer.SetPipeline(presentPipeline);
    renderer.BeginFrame();
    renderer.EndFrame();
}

void SceneRenderer::Clear()
{
    m_drawList.clear();
}

void SceneRenderer::Resize(int x, int y)
{
    viewport = { x, y };
    CreateOffscreenPasses();
}

uint32_t SceneRenderer::Pick(const std::shared_ptr<Scene>& scene, const glm::vec2& pos)
{
    CameraData cameraData{};
    for (const auto eCamera : scene->GetRegistry().view<CameraComponent>())
    {
        auto& camera = scene->GetRegistry().get<CameraComponent>(eCamera);
        auto& transform = scene->GetRegistry().get<TransformComponent>(eCamera);
        if (camera.isPrimary)
        {
            cameraData.position = transform.GetWorldPosition();
            cameraData.view = camera.GetViewMatrix(cameraData.position);
            cameraData.projection = camera.GetProjectionMatrix();
            break;
        }
    }

    auto& am = AssetManager::Get();
    std::vector<DrawCommand> drawList;
    std::vector<UBColorId> colorIds;
    for (const auto eMesh : scene->GetRegistry().view<MeshComponent>())
    {
        auto& transform = scene->GetRegistry().get<TransformComponent>(eMesh);
        auto& mc = scene->GetRegistry().get<MeshComponent>(eMesh);
        auto material = am.GetAsset<Material>(mc.material);
        for (const auto& mesh : am.GetAsset<MeshNode>(mc.mesh)->meshes)
        {
            auto& dc = drawList.emplace_back();
            dc.material = material;
            dc.mesh = mesh;
            dc.transform = transform.GetWorldTransformMatrix();
        }
        auto& tag = scene->GetRegistry().get<TagComponent>(eMesh);
        colorIds.emplace_back().color = IdToColor(tag.colorId) / 255.0f;
    }

    UBCameraData ubCameraData;
    ubCameraData.position = glm::vec4(cameraData.position, 1.0);
    auto projection = cameraData.projection;
    projection[1][1] *= -1;
    ubCameraData.viewProjection = projection * cameraData.view;

    uniformBufferSet->Get(1)->SetData(&ubCameraData, sizeof(ubCameraData));

    renderer.SetPipeline(m_pickingPipeline);
    renderer.BeginFrame();
    std::vector<std::shared_ptr<RendererState>> rendererStates(512);

    auto& transformBuffer = uniformBufferSet->Get(0);
    auto& cameraBuffer = uniformBufferSet->Get(1);
    auto& colorIdBuffer = uniformBufferSet->Get(13);

    uint32_t transformBufferOffset = 0;
    uint32_t colorIdOffset = 0;
    const size_t transformDataSize = Device::Get()->GetAlignedGPUDataSize(sizeof(UBTransformData));
    const size_t colorIdDataSize = Device::Get()->GetAlignedGPUDataSize(sizeof(UBColorId));
    for (int i = 0; i < drawList.size(); i++)
    {
        auto& dc = drawList[i];
        transformBuffer->SetData(&dc.transform, transformDataSize, transformBufferOffset);
        colorIdBuffer->SetData(&colorIds[i], colorIdDataSize, colorIdOffset);

        auto& rs = rendererStates[i];
        rs = RendererCommand::CreateRendererState();
        rs->SetVertexBuffer(transformBuffer, 0, transformBufferOffset, sizeof(UBTransformData));
        rs->SetVertexBuffer(cameraBuffer, 1);
        rs->SetFragmentBuffer(colorIdBuffer, 13, colorIdOffset, sizeof(MaterialData));

        rs->OnUpdate(renderer.GetActivePipeline());
        renderer.EncodeState(rs);
        renderer.Draw(dc.mesh);

        transformBufferOffset += transformDataSize;
        colorIdOffset += colorIdDataSize;
    }
    renderer.EndFrame();

    auto& texture = m_pickingPipeline->GetRenderPassDescriptor().colorAttachments[0].texture;
    auto buffer = texture->Data();
    auto ptr = buffer->Map();
    std::vector<Pixel> pickData(buffer->GetDescriptor().size / sizeof(Pixel));
    memcpy(pickData.data(), ptr, buffer->GetDescriptor().size);
    buffer->UnMap();

    auto pickedPixel = pickData[texture->GetSpecification().width * pos.y + pos.x];
    uint32_t id = ColorToId(pickedPixel);
    return ColorToId(pickedPixel);
}

const std::shared_ptr<GraphicsPipeline>& SceneRenderer::GetPass(PassType type) const
{
    switch (type)
    {
		case PassType::SHADOW:
			return m_shadowPipeline;
        case PassType::UI:
            return uiPipeline;
        case PassType::PBR:
            return pbrPipeline;
        case PassType::SKYBOX:
            return skyboxPipeline;
        case PassType::PRESENT:
            return presentPipeline;
        case PassType::POSTPROCESS:
            return postprocessPipeline;
        default:
        R_CORE_ASSERT(false, "")
    }
}

const std::shared_ptr<Texture>& SceneRenderer::GetFinalImage() const
{
    return pbrPipeline->GetRenderPassDescriptor().colorAttachments.front().texture;
}
