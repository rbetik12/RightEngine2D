#include "RHIHelpers.hpp"

namespace RightEngine
{
	namespace helpers
	{
		ShaderDescriptor CreateShaderDescriptor(std::string_view path, ShaderType type, std::string_view name)
		{
			return {name.data(), path.data(), type};
		}

		TextureDescriptor CreateTextureDescriptor(int width,
		                                          int height,
		                                          TextureType type,
		                                          Format format,
		                                          int componentAmount,
		                                          int mipLevels)
		{
			return {width, height, componentAmount, mipLevels, type, format};
		}

		RenderPassDescriptor CreateRenderPassDescriptor(const glm::ivec2& extent,
		                                                const std::vector<AttachmentDescriptor>& colors,
		                                                const AttachmentDescriptor& depth,
														bool offscreen,
		                                                std::string_view name)
		{
			return { extent, offscreen, colors, depth, name.data()};
		}

		AttachmentDescriptor CreateAttachmentDescriptor(const std::shared_ptr<Texture>& texture, 
														ClearValue clearValue,
														AttachmentLoadOperation load, 
														AttachmentStoreOperation store)
		{
			return { texture, clearValue, load, store };
		}
	}
}
