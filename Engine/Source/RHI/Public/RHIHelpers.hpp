#pragma once

#include "ShaderProgramDescriptor.hpp"
#include "TextureDescriptor.hpp"
#include "GraphicsPipelineDescriptor.hpp"

namespace RightEngine
{
	namespace helpers
	{
		ShaderDescriptor CreateShaderDescriptor(std::string_view path, ShaderType type, std::string_view name = "");
		TextureDescriptor CreateTextureDescriptor(int width,
		                                          int height,
		                                          TextureType type,
		                                          Format format,
		                                          int componentAmount = 0,
		                                          int mipLevels = 1);
		RenderPassDescriptor CreateRenderPassDescriptor(const glm::ivec2& extent,
		                                                const std::vector<AttachmentDescriptor>& colors,
		                                                const AttachmentDescriptor& depth,
														bool offscreen = true,
		                                                std::string_view name = "");
		AttachmentDescriptor CreateAttachmentDescriptor(const std::shared_ptr<Texture>& texture,
														ClearValue clearValue = {},
														AttachmentLoadOperation load = AttachmentLoadOperation::CLEAR,
														AttachmentStoreOperation store = AttachmentStoreOperation::STORE);
	}
}
