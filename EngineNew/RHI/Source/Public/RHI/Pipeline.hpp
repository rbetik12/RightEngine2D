#pragma once

#include <RHI/Config.hpp>
#include <cstdint>

namespace rhi
{

enum class CompareOp : uint8_t
{
    LESS = 0,
    LESS_OR_EQUAL,
    GREATER
};

enum class CullMode : uint8_t
{
    BACK = 0,
    FRONT
};

struct PipelineDescriptor
{
	CompareOp	m_depthCompareOp = CompareOp::LESS;
	CullMode	m_cullMode = CullMode::BACK;
};

class RHI_API Pipeline
{
public:
	virtual ~Pipeline() = default;

protected:
	PipelineDescriptor m_descriptor;

	Pipeline(const PipelineDescriptor& descriptor) : m_descriptor(descriptor)
	{}
};

}