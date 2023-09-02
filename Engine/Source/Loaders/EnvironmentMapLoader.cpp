#include "EnvironmentMapLoader.hpp"
#include "TextureLoader.hpp"
#include "MeshBuilder.hpp"
#include "Shader.hpp"
#include "Renderer.hpp"
#include "RendererCommand.hpp"
#include "String.hpp"
#include "AssetManager.hpp"
#include "GraphicsPipeline.hpp"
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

using namespace RightEngine;

namespace
{
    const uint32_t envTexWidth = 2048;
    const uint32_t envTexHeight = 2048;
    const uint32_t irradianceTexWidth = 64;
    const uint32_t irradianceTexHeight = 64;
    const uint32_t prefilterTexWidth = 128;
    const uint32_t prefilterTexHeight = 128;
    const uint32_t lutTexWidth = 512;
    const uint32_t lutTexHeight = 512;

    const glm::mat4 captureViews[] =
            {
                    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f),
                                glm::vec3(1.0f, 0.0f, 0.0f),
                                glm::vec3(0.0f, -1.0f, 0.0f)),
                    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f),
                                glm::vec3(-1.0f, 0.0f, 0.0f),
                                glm::vec3(0.0f, -1.0f, 0.0f)),
                    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f),
                                glm::vec3(0.0f, 1.0f, 0.0f),
                                glm::vec3(0.0f, 0.0f, 1.0f)),
                    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f),
                                glm::vec3(0.0f, -1.0f, 0.0f),
                                glm::vec3(0.0f, 0.0f, -1.0f)),
                    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f),
                                glm::vec3(0.0f, 0.0f, 1.0f),
                                glm::vec3(0.0f, -1.0f, 0.0f)),
                    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f),
                                glm::vec3(0.0f, 0.0f, -1.0f),
                                glm::vec3(0.0f, -1.0f, 0.0f))
            };
    std::shared_ptr<MeshComponent> cube;
    const auto projectionMatrix = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    const uint32_t maxMipLevels = 8;

    std::string GetTextureName(const std::string& path)
    {
        const auto splittedPath = String::Split(path, "/");
        return splittedPath.back();
    }

    const float cubeVertexData[] = {
        // [position 3] [normal 3] [texture coodinate 2]
        // back face
        -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
        1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
        1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
        -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,
        // front face
        -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        // left face
        -1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,  // top-right
        -1.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f,  // top-left
        -1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,  // bottom-left
        -1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,  // bottom-left
        -1.0f, -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // bottom-right
        -1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,  // top-right
        // right face
        1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        // bottom face
        -1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,
        1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f,
        1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        -1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f,
        -1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,
        // top face
        -1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
        1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
        -1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
    };

    const float quadVertexData[] = {
        // positions        // texture Coords
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
        -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
    };
}

EnvironmentMapLoader::EnvironmentMapLoader()
{
//    cube = MeshBuilder::CubeGeometry();
}

AssetHandle EnvironmentMapLoader::Load(const std::string& path, bool flipVertically)
{
    return _Load(path, xg::newGuid(), flipVertically);
}

void EnvironmentMapLoader::ComputeEnvironmentMap()
{
    auto& assetManager = AssetManager::Get();
    auto loader = assetManager.GetLoader<TextureLoader>();
    auto textureHandle = loader->Load(m_loaderContext.path, {});
    const auto equirectMap = assetManager.GetAsset<Texture>(textureHandle);

    ShaderProgramDescriptor shaderProgramDescriptor;
    ShaderDescriptor vertexShader;
    vertexShader.path = "/Engine/Shaders/Utils/envmap_to_cubemap.vert";
    vertexShader.type = ShaderType::VERTEX;
    ShaderDescriptor fragmentShader;
    fragmentShader.path = "/Engine/Shaders/Utils/envmap_to_cubemap.frag";
    fragmentShader.type = ShaderType::FRAGMENT;
    shaderProgramDescriptor.shaders = { vertexShader, fragmentShader };
    VertexBufferLayout layout;
    layout.Push<glm::vec3>();
    layout.Push<glm::vec3>();
    layout.Push<glm::vec2>();
    shaderProgramDescriptor.layout = layout;
    shaderProgramDescriptor.reflection.textures = { 1 };
    shaderProgramDescriptor.reflection.buffers[{ 0, ShaderType::VERTEX }] = BufferType::UNIFORM;
    const auto shader = Device::Get()->CreateShader(shaderProgramDescriptor);

    TextureDescriptor cubeDesc;
    cubeDesc.type = TextureType::CUBEMAP;
    cubeDesc.width = envTexWidth;
    cubeDesc.height = envTexHeight;
    cubeDesc.format = Format::RGBA16_SFLOAT;
    cubeDesc.componentAmount = 3;
    const auto cubemap = Device::Get()->CreateTexture(cubeDesc, {});

    TextureDescriptor colorAttachmentDesc{};
    colorAttachmentDesc.format = Format::RGBA16_SFLOAT;
    colorAttachmentDesc.type = TextureType::TEXTURE_2D;
    colorAttachmentDesc.width = envTexWidth;
    colorAttachmentDesc.height = envTexHeight;
    const auto colorAttachment = Device::Get()->CreateTexture(colorAttachmentDesc, {});
    TextureDescriptor depthAttachmentDesc{};
    depthAttachmentDesc.format = Format::D32_SFLOAT_S8_UINT;
    depthAttachmentDesc.type = TextureType::TEXTURE_2D;
    depthAttachmentDesc.width = envTexWidth;
    depthAttachmentDesc.height = envTexHeight;
    const auto depthAttachment = Device::Get()->CreateTexture(depthAttachmentDesc, {});

    RenderPassDescriptor renderPassDescriptor{};
    renderPassDescriptor.extent = { envTexWidth, envTexHeight };
    renderPassDescriptor.offscreen = true;
    AttachmentDescriptor depth{};
    depth.loadOperation = AttachmentLoadOperation::CLEAR;
    depth.texture = depthAttachment;
    AttachmentDescriptor color{};
    color.texture = colorAttachment;
    color.loadOperation = AttachmentLoadOperation::CLEAR;
    renderPassDescriptor.colorAttachments = { color };
    renderPassDescriptor.depthStencilAttachment = { depth };

    GraphicsPipelineDescriptor pipelineDescriptor;
    pipelineDescriptor.shader = shader;

    const auto pipeline = Device::Get()->CreateGraphicsPipeline(pipelineDescriptor, renderPassDescriptor);

    Renderer renderer;
    renderer.SetPipeline(pipeline);

    const auto rendererState = RendererCommand::CreateRendererState();
    struct Camera
    {
        glm::mat4 view;
        glm::mat4 projection;
    };
    BufferDescriptor bufferDescriptor{};
    bufferDescriptor.type = BufferType::VERTEX;
    bufferDescriptor.size = sizeof(cubeVertexData);
    bufferDescriptor.memoryType = MemoryType::CPU_GPU;
    const auto vertexBuffer = Device::Get()->CreateBuffer(bufferDescriptor, cubeVertexData);

    bufferDescriptor.type = BufferType::UNIFORM;
    bufferDescriptor.size = sizeof(Camera);
    bufferDescriptor.memoryType = MemoryType::CPU_GPU;
    const auto buffer = Device::Get()->CreateBuffer(bufferDescriptor, nullptr);
    rendererState->SetVertexBuffer(buffer, 0);
    rendererState->SetTexture(equirectMap, 1);

    SamplerDescriptor samplerDescriptor{};
    const auto sampler = Device::Get()->CreateSampler(samplerDescriptor);
    TextureDescriptor environmentCubemapDesc;
    environmentCubemapDesc.type = TextureType::CUBEMAP;
    environmentCubemapDesc.componentAmount = 4;
    environmentCubemapDesc.format = Format::RGBA16_SFLOAT;
    environmentCubemapDesc.height = envTexHeight;
    environmentCubemapDesc.width = envTexWidth;
    const auto environmentCubemap = Device::Get()->CreateTexture(environmentCubemapDesc, {});
    
    environmentCubemap->SetSampler(sampler);
    colorAttachment->SetSampler(sampler);
    equirectMap->SetSampler(sampler);
    
    TextureCopy src;
    src.usage = SHADER_READ_ONLY;
    src.layerNum = 0;
    src.mipLevel = 0;

    for (int i = 0; i < 6; i++)
    {
        Camera camera;
        camera.view = captureViews[i];
        camera.projection = projectionMatrix;
        void* ptr = buffer->Map();
        memcpy(ptr, &camera, sizeof(Camera));
        buffer->UnMap();
        rendererState->OnUpdate(pipeline);
        renderer.BeginFrame();
        renderer.EncodeState(rendererState);
        renderer.Draw(vertexBuffer);
        renderer.EndFrame();
        
        TextureCopy dst;
        dst.usage = SHADER_READ_ONLY;
        dst.layerNum = i;
        dst.mipLevel = 0;
        environmentCubemap->CopyFrom(colorAttachment, src, dst);
    }
    
    m_environmentContext->envMap = environmentCubemap;
    m_environmentContext->equirectangularTexture = equirectMap;
}

void EnvironmentMapLoader::ComputeIrradianceMap()
{
    ShaderProgramDescriptor shaderProgramDescriptor;
    ShaderDescriptor vertexShader;
    vertexShader.path = "/Engine/Shaders/Utils/envmap_to_irr_map.vert";
    vertexShader.type = ShaderType::VERTEX;
    ShaderDescriptor fragmentShader;
    fragmentShader.path = "/Engine/Shaders/Utils/envmap_to_irr_map.frag";
    fragmentShader.type = ShaderType::FRAGMENT;
    shaderProgramDescriptor.shaders = { vertexShader, fragmentShader };
    VertexBufferLayout layout;
    layout.Push<glm::vec3>();
    layout.Push<glm::vec3>();
    layout.Push<glm::vec2>();
    shaderProgramDescriptor.layout = layout;
    shaderProgramDescriptor.reflection.textures = { 1 };
    shaderProgramDescriptor.reflection.buffers[{ 0, ShaderType::VERTEX }] = BufferType::UNIFORM;
    const auto shader = Device::Get()->CreateShader(shaderProgramDescriptor);

    TextureDescriptor colorAttachmentDesc{};
    colorAttachmentDesc.format = Format::RGBA16_SFLOAT;
    colorAttachmentDesc.type = TextureType::TEXTURE_2D;
    colorAttachmentDesc.width = irradianceTexWidth;
    colorAttachmentDesc.height = irradianceTexHeight;
    const auto colorAttachment = Device::Get()->CreateTexture(colorAttachmentDesc, {});
    TextureDescriptor depthAttachmentDesc{};
    depthAttachmentDesc.format = Format::D32_SFLOAT_S8_UINT;
    depthAttachmentDesc.type = TextureType::TEXTURE_2D;
    depthAttachmentDesc.width = irradianceTexWidth;
    depthAttachmentDesc.height = irradianceTexHeight;
    const auto depthAttachment = Device::Get()->CreateTexture(depthAttachmentDesc, {});

    RenderPassDescriptor renderPassDescriptor{};
    renderPassDescriptor.extent = { irradianceTexWidth, irradianceTexHeight };
    renderPassDescriptor.offscreen = true;
    AttachmentDescriptor depth{};
    depth.loadOperation = AttachmentLoadOperation::CLEAR;
    depth.texture = depthAttachment;
    AttachmentDescriptor color{};
    color.texture = colorAttachment;
    color.loadOperation = AttachmentLoadOperation::CLEAR;
    renderPassDescriptor.colorAttachments = { color };
    renderPassDescriptor.depthStencilAttachment = { depth };


    GraphicsPipelineDescriptor pipelineDescriptor;
    pipelineDescriptor.shader = shader;

    const auto pipeline = Device::Get()->CreateGraphicsPipeline(pipelineDescriptor, renderPassDescriptor);

    Renderer renderer;
    renderer.SetPipeline(pipeline);

    const auto rendererState = RendererCommand::CreateRendererState();
    struct Camera
    {
        glm::mat4 view;
        glm::mat4 projection;
    };
    BufferDescriptor bufferDescriptor{};
    bufferDescriptor.type = BufferType::VERTEX;
    bufferDescriptor.size = sizeof(cubeVertexData);
    bufferDescriptor.memoryType = MemoryType::CPU_GPU;
    const auto vertexBuffer = Device::Get()->CreateBuffer(bufferDescriptor, cubeVertexData);

    bufferDescriptor.type = BufferType::UNIFORM;
    bufferDescriptor.size = sizeof(Camera);
    bufferDescriptor.memoryType = MemoryType::CPU_GPU;
    const auto buffer = Device::Get()->CreateBuffer(bufferDescriptor, nullptr);
    rendererState->SetVertexBuffer(buffer, 0);
    rendererState->SetTexture(m_environmentContext->envMap, 1);

    SamplerDescriptor samplerDescriptor{};
    const auto sampler = Device::Get()->CreateSampler(samplerDescriptor);
    TextureDescriptor irradianceCubemapDesc;
    irradianceCubemapDesc.type = TextureType::CUBEMAP;
    irradianceCubemapDesc.componentAmount = 4;
    irradianceCubemapDesc.format = Format::RGBA16_SFLOAT;
    irradianceCubemapDesc.height = irradianceTexHeight;
    irradianceCubemapDesc.width = irradianceTexWidth;
    const auto irradianceCubemap = Device::Get()->CreateTexture(irradianceCubemapDesc, {});

    irradianceCubemap->SetSampler(sampler);
    colorAttachment->SetSampler(sampler);

    TextureCopy src;
    src.usage = SHADER_READ_ONLY;
    src.layerNum = 0;
    src.mipLevel = 0;

    for (int i = 0; i < 6; i++)
    {
        Camera camera;
        camera.view = captureViews[i];
        camera.projection = projectionMatrix;
        void* ptr = buffer->Map();
        memcpy(ptr, &camera, sizeof(Camera));
        buffer->UnMap();
        rendererState->OnUpdate(pipeline);
        renderer.BeginFrame();
        renderer.EncodeState(rendererState);
        renderer.Draw(vertexBuffer);
        renderer.EndFrame();

        TextureCopy dst;
        dst.usage = SHADER_READ_ONLY;
        dst.layerNum = i;
        dst.mipLevel = 0;
        irradianceCubemap->CopyFrom(colorAttachment, src, dst);
    }


    m_environmentContext->irradianceMap = irradianceCubemap;
    R_CORE_TRACE("Finished computing irradiance map for texture \"{0}\"", m_loaderContext.path);
}

void EnvironmentMapLoader::ComputeRadianceMap()
{
    ShaderProgramDescriptor shaderProgramDescriptor;
    ShaderDescriptor vertexShader;
    vertexShader.path = "/Engine/Shaders/Utils/envmap_to_radiance_map.vert";
    vertexShader.type = ShaderType::VERTEX;
    ShaderDescriptor fragmentShader;
    fragmentShader.path = "/Engine/Shaders/Utils/envmap_to_radiance_map.frag";
    fragmentShader.type = ShaderType::FRAGMENT;
    shaderProgramDescriptor.shaders = { vertexShader, fragmentShader };
    VertexBufferLayout layout;
    layout.Push<glm::vec3>();
    layout.Push<glm::vec3>();
    layout.Push<glm::vec2>();
    shaderProgramDescriptor.layout = layout;
    shaderProgramDescriptor.reflection.textures = { 1 };
    shaderProgramDescriptor.reflection.buffers[{ 0, ShaderType::VERTEX }] = BufferType::UNIFORM;
    shaderProgramDescriptor.reflection.buffers[{ 2, ShaderType::FRAGMENT }] = BufferType::UNIFORM;
    const auto shader = Device::Get()->CreateShader(shaderProgramDescriptor);
    
    TextureDescriptor colorAttachmentDesc{};
    colorAttachmentDesc.format = Format::RGBA16_SFLOAT;
    colorAttachmentDesc.type = TextureType::TEXTURE_2D;
    colorAttachmentDesc.width = prefilterTexWidth;
    colorAttachmentDesc.height = prefilterTexHeight;
    auto colorAttachment = Device::Get()->CreateTexture(colorAttachmentDesc, {});
    TextureDescriptor depthAttachmentDesc{};
    depthAttachmentDesc.format = Format::D32_SFLOAT_S8_UINT;
    depthAttachmentDesc.type = TextureType::TEXTURE_2D;
    depthAttachmentDesc.width = prefilterTexWidth;
    depthAttachmentDesc.height = prefilterTexHeight;
    const auto depthAttachment = Device::Get()->CreateTexture(depthAttachmentDesc, {});

    RenderPassDescriptor renderPassDescriptor{};
    renderPassDescriptor.extent = { prefilterTexWidth, prefilterTexHeight };
    renderPassDescriptor.offscreen = true;
    AttachmentDescriptor depth{};
    depth.loadOperation = AttachmentLoadOperation::CLEAR;
    depth.texture = depthAttachment;
    AttachmentDescriptor color{};
    color.texture = colorAttachment;
    color.loadOperation = AttachmentLoadOperation::CLEAR;
    renderPassDescriptor.colorAttachments = { color };
    renderPassDescriptor.depthStencilAttachment = { depth };
    
    GraphicsPipelineDescriptor pipelineDescriptor;
    pipelineDescriptor.shader = shader;

    const auto pipeline = Device::Get()->CreateGraphicsPipeline(pipelineDescriptor, renderPassDescriptor);

    Renderer renderer;
    renderer.SetPipeline(pipeline);
    const auto rendererState = RendererCommand::CreateRendererState();
    
    struct Camera
    {
        glm::mat4 view;
        glm::mat4 projection;
    };
    float roughness;
    
    BufferDescriptor bufferDescriptor{};
    bufferDescriptor.type = BufferType::VERTEX;
    bufferDescriptor.size = sizeof(cubeVertexData);
    bufferDescriptor.memoryType = MemoryType::CPU_GPU;
    const auto vertexBuffer = Device::Get()->CreateBuffer(bufferDescriptor, cubeVertexData);

    bufferDescriptor.type = BufferType::UNIFORM;
    bufferDescriptor.size = sizeof(Camera);
    bufferDescriptor.memoryType = MemoryType::CPU_GPU;
    const auto buffer = Device::Get()->CreateBuffer(bufferDescriptor, nullptr);
    bufferDescriptor.type = BufferType::UNIFORM;
    bufferDescriptor.size = sizeof(roughness);
    bufferDescriptor.memoryType = MemoryType::CPU_GPU;
    const auto roughnessBuffer = Device::Get()->CreateBuffer(bufferDescriptor, nullptr);
    
    rendererState->SetVertexBuffer(buffer, 0);
    rendererState->SetTexture(m_environmentContext->envMap, 1);
    rendererState->SetFragmentBuffer(roughnessBuffer, 2);

    SamplerDescriptor samplerDescriptor{};
    samplerDescriptor.maxLod = 9.0f;
    const auto sampler = Device::Get()->CreateSampler(samplerDescriptor);
    TextureDescriptor prefilterCubemapDesc;
    prefilterCubemapDesc.type = TextureType::CUBEMAP;
    prefilterCubemapDesc.componentAmount = 4;
    prefilterCubemapDesc.format = Format::RGBA16_SFLOAT;
    prefilterCubemapDesc.height = prefilterTexWidth;
    prefilterCubemapDesc.width = prefilterTexHeight;
    prefilterCubemapDesc.mipLevels = maxMipLevels;
    const auto prefilterCubemap = Device::Get()->CreateTexture(prefilterCubemapDesc, {});

    prefilterCubemap->SetSampler(sampler);
    colorAttachment->SetSampler(sampler);
    
    for (int mipLevel = 0; mipLevel < maxMipLevels; mipLevel++)
    {
        uint32_t mipWidth = prefilterTexWidth * std::pow(0.5, mipLevel);
        uint32_t mipHeight = prefilterTexHeight * std::pow(0.5, mipLevel);
        pipeline->Resize(mipWidth, mipHeight);
        colorAttachment = pipeline->GetRenderPassDescriptor().colorAttachments.front().texture;
        roughness = static_cast<float>(mipLevel) / static_cast<float>(maxMipLevels - 1);
        auto roughnessPtr = roughnessBuffer->Map();
        memcpy(roughnessPtr, &roughness, sizeof(roughness));
        roughnessBuffer->UnMap();
        
        for (int i = 0; i < 6; i++)
        {
            Camera camera;
            camera.view = captureViews[i];
            camera.projection = projectionMatrix;
            
            auto ptr = buffer->Map();
            memcpy(ptr, &camera, sizeof(camera));
            buffer->UnMap();
            
            rendererState->OnUpdate(pipeline);
            renderer.BeginFrame();
            renderer.EncodeState(rendererState);
            renderer.Draw(vertexBuffer);
            renderer.EndFrame();
            
            TextureCopy src;
            src.usage = SHADER_READ_ONLY;
            src.layerNum = 0;
            src.mipLevel = 0;
            
            TextureCopy dst;
            dst.usage = SHADER_READ_ONLY;
            dst.layerNum = i;
            dst.mipLevel = mipLevel;
            
            
            prefilterCubemap->CopyFrom(colorAttachment, src, dst);
        }
    }
    
    m_environmentContext->prefilterMap = prefilterCubemap;
    R_CORE_TRACE("Finished computing irradiance map for texture \"{0}\"", m_loaderContext.path);
}

void EnvironmentMapLoader::ComputeLUT()
{
    static bool computed = false;

    if (computed)
    {
        m_environmentContext->brdfLut = m_lut;
        return;
    }
    computed = true;

    ShaderProgramDescriptor shaderProgramDescriptor;
    ShaderDescriptor vertexShader;
    vertexShader.path = "/Engine/Shaders/Utils/brdf.vert";
    vertexShader.type = ShaderType::VERTEX;
    ShaderDescriptor fragmentShader;
    fragmentShader.path = "/Engine/Shaders/Utils/brdf.frag";
    fragmentShader.type = ShaderType::FRAGMENT;
    shaderProgramDescriptor.shaders = { vertexShader, fragmentShader };
    VertexBufferLayout layout;
    layout.Push<glm::vec3>();
    layout.Push<glm::vec2>();
    shaderProgramDescriptor.layout = layout;
    const auto shader = Device::Get()->CreateShader(shaderProgramDescriptor);
    
    TextureDescriptor colorAttachmentDesc{};
    colorAttachmentDesc.format = Format::RG16_SFLOAT;
    colorAttachmentDesc.type = TextureType::TEXTURE_2D;
    colorAttachmentDesc.width = lutTexWidth;
    colorAttachmentDesc.height = lutTexHeight;
    auto colorAttachment = Device::Get()->CreateTexture(colorAttachmentDesc, {});
    TextureDescriptor depthAttachmentDesc{};
    depthAttachmentDesc.format = Format::D32_SFLOAT_S8_UINT;
    depthAttachmentDesc.type = TextureType::TEXTURE_2D;
    depthAttachmentDesc.width = lutTexWidth;
    depthAttachmentDesc.height = lutTexHeight;
    const auto depthAttachment = Device::Get()->CreateTexture(depthAttachmentDesc, {});
    
    RenderPassDescriptor renderPassDescriptor{};
    renderPassDescriptor.extent = { lutTexWidth, lutTexHeight };
    renderPassDescriptor.offscreen = true;
    AttachmentDescriptor depth{};
    depth.loadOperation = AttachmentLoadOperation::CLEAR;
    depth.texture = depthAttachment;
    AttachmentDescriptor color{};
    color.texture = colorAttachment;
    color.loadOperation = AttachmentLoadOperation::CLEAR;
    renderPassDescriptor.colorAttachments = { color };
    renderPassDescriptor.depthStencilAttachment = { depth };
    
    GraphicsPipelineDescriptor pipelineDescriptor;
    pipelineDescriptor.shader = shader;
    
    const auto pipeline = Device::Get()->CreateGraphicsPipeline(pipelineDescriptor, renderPassDescriptor);

    Renderer renderer;
    renderer.SetPipeline(pipeline);
    const auto rendererState = RendererCommand::CreateRendererState();
    
    SamplerDescriptor samplerDescriptor{};
    const auto sampler = Device::Get()->CreateSampler(samplerDescriptor);
    colorAttachment->SetSampler(sampler);
    
    BufferDescriptor bufferDescriptor{};
    bufferDescriptor.type = BufferType::VERTEX;
    bufferDescriptor.size = sizeof(quadVertexData);
    bufferDescriptor.memoryType = MemoryType::CPU_GPU;
    const auto vertexBuffer = Device::Get()->CreateBuffer(bufferDescriptor, quadVertexData);
    
    rendererState->OnUpdate(pipeline);
    renderer.BeginFrame();
    renderer.EncodeState(rendererState);
    renderer.Draw(vertexBuffer);
    renderer.EndFrame();
    
    m_environmentContext->brdfLut = colorAttachment;
    m_lut = colorAttachment;
    R_CORE_TRACE("Finished computing BRDF map for texture \"{0}\"", m_loaderContext.path);
}

AssetHandle EnvironmentMapLoader::FinishLoading(const xg::Guid& guid)
{
    R_CORE_ASSERT(manager, "")
    return manager->CacheAsset(m_environmentContext, m_loaderContext.path, AssetType::ENVIRONMENT_MAP, guid);
}

AssetHandle EnvironmentMapLoader::_Load(const std::string& path, const xg::Guid& guid, const bool flipVertically)
{
    R_CORE_ASSERT(manager, "");

    const auto asset = manager->GetAsset<EnvironmentContext>(path);
    if (asset)
    {
        return { asset->guid };
    }

    m_environmentContext = std::make_shared<EnvironmentContext>();
    m_loaderContext.path = path;
    m_environmentContext->name = GetTextureName(path);
    ComputeEnvironmentMap();
    ComputeIrradianceMap();
    ComputeRadianceMap();
    ComputeLUT();
    return FinishLoading(guid);
}

AssetHandle EnvironmentMapLoader::LoadWithGUID(const std::string& path, const xg::Guid& guid, bool flipVertically)
{
    return _Load(path, guid, flipVertically);
}
