#pragma once

#include <RHI/Config.hpp>
#include <RHI/TextureDescriptor.hpp>

namespace rhi
{
	class RHI_API Texture
	{
	public:
		virtual ~Texture() = default;

	protected:
		TextureDescriptor m_descriptor;

		Texture(const TextureDescriptor& desc) : m_descriptor(desc)
		{}
	};
}