#pragma once

#include <Engine/Service/Resource/Resource.hpp>
#include <Engine/Service/Resource/Loader.hpp>
#include <RHI/Texture.hpp>
#include <taskflow/taskflow.hpp>

namespace engine
{

class TextureResource;

class ENGINE_API TextureLoader final : public Loader
{
	RTTR_ENABLE(Loader)
public:
	TextureLoader();

	virtual void Update() override;

	virtual ResPtr<Resource> Load(const fs::path& path) override;

private:
	bool Load(const ResPtr<TextureResource>& resource);

	std::mutex							m_mutex;
	eastl::vector<tf::Future<void>>		m_loadingTasks;
};

class ENGINE_API TextureResource final : public Resource
{
public:
	TextureResource(const io::fs::path& path);
	virtual ~TextureResource() {}

	const ResPtr<rhi::Texture>& Texture() const { return m_texture; }

	friend class TextureLoader;

private:
	ResPtr<rhi::Texture> m_texture;
};

} // engine