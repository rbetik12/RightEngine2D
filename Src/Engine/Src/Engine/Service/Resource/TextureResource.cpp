#include <Engine/Service/Resource/TextureResource.hpp>
#include <Engine/Engine.hpp>
#include <Engine/Registration.hpp>
#include <Engine/Service/ThreadService.hpp>
#include <Engine/Service/Filesystem/File.hpp>
#include <Engine/Service/Render/RenderService.hpp>
#include <RHI/Helpers.hpp>
#include <stb_image.h>

RTTR_REGISTRATION
{
	using namespace engine::registration;

	ResourceLoader<engine::TextureLoader>("engine::TextureLoader");
}

namespace
{

bool IsHDR(const engine::io::fs::path& path)
{
	const std::string hdr = ".hdr";
	bool isHdr = false;
	isHdr |= stbi_is_hdr(path.generic_u8string().c_str()) > 0;
	isHdr |= path.extension().generic_u8string().find(hdr) != std::string::npos;
	return isHdr;
}

rhi::Format ChooseTextureFormat(int componentsAmount, bool hdr)
{
	if (hdr)
	{
		return rhi::Format::RGBA32_SFLOAT;
	}

	switch (componentsAmount)
	{
	case 1:
		return rhi::Format::R8_UINT;
	case 3:
		return rhi::Format::RGB8_UINT;
	case 4:
		return rhi::Format::RGBA8_UINT;
	default:
		ENGINE_ASSERT(false);
		return rhi::Format::NONE;
	}
}

} // unnamed

namespace engine
{

TextureLoader::TextureLoader()
{
	stbi_set_flip_vertically_on_load(false);
}

void TextureLoader::Update()
{
	PROFILER_CPU_ZONE;
}

ResPtr<Resource> TextureLoader::Load(const fs::path& path)
{
	std::lock_guard l(m_mutex);

	if (auto res = Get(path))
	{
		return res;
	}

	auto resource = MakeResPtr<TextureResource>(path);
	resource->m_status = Resource::Status::LOADING;
	m_cache[path] = resource;

	auto& ts = Instance().Service<ThreadService>();

	ts.AddBackgroundTask([this, resource]()
		{
			PROFILER_CPU_ZONE_NAME("Load texture");
			const auto result = Load(resource);
			resource->m_status = result ? Resource::Status::READY : Resource::Status::FAILED;
		});

	return resource;
}

ResPtr<Resource> TextureLoader::Get(const fs::path& path) const
{
	if (const auto it = m_cache.find(path); it != m_cache.end())
	{
		return it->second;
	}
	return {};
}

bool TextureLoader::Load(const ResPtr<TextureResource>& resource)
{
	io::File file(resource->m_srcPath);

	if (!file.Read())
	{
		return false;
	}

	int componentAmount;
	int width;
	int height;

	if (!stbi_info_from_memory(static_cast<stbi_uc*>(file.Raw()), static_cast<int>(file.Size()),
		&width, &height, &componentAmount))
	{
		return false;
	}

	const int desiredComponentAmount = componentAmount == 3 ? 4 : componentAmount;
	const auto isHdr = IsHDR(resource->SourcePath());
	void* buffer = nullptr;

	if (isHdr)
	{
		buffer = stbi_loadf_from_memory(static_cast<stbi_uc*>(file.Raw()),
			static_cast<int>(file.Size()),
			&width,
			&height,
			&componentAmount,
			desiredComponentAmount);
	}
	else
	{
		buffer = stbi_load_from_memory(static_cast<stbi_uc*>(file.Raw()),
			static_cast<int>(file.Size()),
			&width,
			&height,
			&componentAmount,
			desiredComponentAmount);
	}

	if (!buffer)
	{
		return false;
	}

	rhi::TextureDescriptor descriptor{};
	descriptor.m_width = static_cast<uint16_t>(width);
	descriptor.m_height = static_cast<uint16_t>(height);
	descriptor.m_format = ChooseTextureFormat(desiredComponentAmount, isHdr);
	descriptor.m_type = rhi::TextureType::TEXTURE_2D;

	auto& rs = Instance().Service<RenderService>();
	resource->m_texture = rs.CreateTexture(descriptor, {}, buffer);

	stbi_image_free(buffer);
	core::log::debug("[TextureLoader] Successfully loaded texture: '{}' ({}x{}) '{}'", resource->SourcePath().generic_u8string(),
		resource->m_texture->Descriptor().m_width,
		resource->m_texture->Descriptor().m_height,
		rhi::helpers::FormatToString(resource->m_texture->Descriptor().m_format)
		);

	return true;
}

TextureResource::TextureResource(const io::fs::path& path) : Resource(path)
{
}

} // engine