#pragma once

#include <cstdint>

namespace rhi
{

class Shader;
class RenderPass;

enum class CompareOp : uint8_t
{
    LESS = 0,
    LESS_OR_EQUAL,
    GREATER
};

enum class CullMode : uint8_t
{
    NONE = 0,
    BACK,
    FRONT
};

struct PipelineDescriptor
{
    std::shared_ptr<Shader>         m_shader;
    std::shared_ptr<RenderPass>     m_pass; // ignored in compute
    CompareOp                       m_depthCompareOp = CompareOp::LESS; // ignored in compute
    CullMode                        m_cullMode = CullMode::BACK; // ignored in compute
    bool                            m_offscreen = true; // ignored in compute
    bool                            m_compute = false;
};

}