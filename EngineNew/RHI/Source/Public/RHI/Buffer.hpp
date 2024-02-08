#pragma once

#include <RHI/Config.hpp>
#include <RHI/BufferDescriptor.hpp>
#include <Core/Type.hpp>

namespace rhi
{
	class RHI_API Buffer : public core::NonCopyable
	{
	public:
		Buffer(const BufferDescriptor& descriptor) : m_descriptor(descriptor)
		{}

		virtual ~Buffer() = default;

		virtual void*	Map() const = 0;
		virtual void	UnMap() const = 0;

		const BufferDescriptor& Descriptor() const
		{ return m_descriptor; }

	protected:
		BufferDescriptor m_descriptor;
	};
}
