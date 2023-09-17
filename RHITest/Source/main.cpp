#include <cassert>
#include <iostream>
#include <EASTL/vector.h>
#include <RHI/IContext.hpp>
#include <RHI/Device.hpp>
#include <GLFW/glfw3.h>
#pragma warning(push)
#pragma warning(disable : 4464)
#include <glslang/Public/ShaderLang.h>
#pragma warning(pop)

struct Vertex
{
    glm::vec2 pos;
    glm::vec3 color;
};

const std::vector<Vertex> vertices = 
{
    {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
    {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
};

constexpr std::string_view vertexShaderCode =   "#version 430 core\n"
												"layout(location = 0) in vec2 aPosition;"
												"layout(location = 1) in vec3 aColor;"
												"layout(binding = 0) uniform UBTransformData { mat4 u_Transform; };"
												"layout(binding = 1) uniform UBCameraData { mat4 u_ViewProjection; vec4 u_CameraPosition; };"
												"void main() { vec3 worldPos = vec3(u_Transform * vec4(aPosition, 1.0, 1.0)); gl_Position = u_ViewProjection * vec4(worldPos, 1.0); }"
;

int main(int argc, char* argv[])
{
    assert(glfwInit());

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(1920, 1080, "RHI Test", nullptr, nullptr);

    // Setup vulkan
    rhi::vulkan::VulkanInitContext ctx;
    ctx.m_surfaceConstructor = [&](VkInstance instance)
    {
        VkSurfaceKHR surface = nullptr;
        glfwCreateWindowSurface(instance, window, nullptr, &surface);
        return surface;
    };

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    eastl::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
	extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

#ifdef R_APPLE
    extensions.push_back("VK_KHR_portability_enumeration");
    extensions.push_back("VK_KHR_get_physical_device_properties2");
#endif
    ctx.m_requiredExtensions = std::move(extensions);

    auto vulkanContext = rhi::vulkan::CreateContext(std::move(ctx));
    auto device = rhi::Device::Create(vulkanContext);

    rhi::BufferDescriptor bufferDesc;
    bufferDesc.m_type = rhi::BufferType::VERTEX;
    bufferDesc.m_memoryType = rhi::MemoryType::CPU_GPU;
    bufferDesc.m_size = vertices.size() * sizeof(Vertex);
    auto buffer = device->CreateBuffer(bufferDesc, vertices.data());

    glslang::InitializeProcess();
    auto shaderCompiler = device->CreateShaderCompiler();
    auto shaderData = shaderCompiler->CompileText(vertexShaderCode, rhi::ShaderType::VERTEX, "basic.vert");
    RHI_ASSERT(shaderData.m_valid);

    rhi::ShaderDescriptor vertexShaderDesc;
    vertexShaderDesc.m_blob = shaderData.m_compiledShader;
    vertexShaderDesc.m_type = rhi::ShaderType::VERTEX;
    vertexShaderDesc.m_reflection = std::move(shaderData.m_reflection);
    const auto vertexShader = device->CreateShader(vertexShaderDesc);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    glslang::FinalizeProcess();
    glfwDestroyWindow(window);
	return 0;
}