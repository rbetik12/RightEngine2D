#pragma once

#include <Engine/Service/IService.hpp>
#include <rttr/registration>

namespace engine::registration
{

template<typename T>
class ENGINE_API Service
{
public:
	Service(rttr::string_view name) : m_class(name)
	{
		static_assert(std::is_base_of_v<IService, T>, "T must be derived of engine::IService");

		m_class.constructor();
	}

	~Service()
	{
		m_class(std::move(m_meta));
	}

	void Domain(Domain domain)
	{
		m_meta.m_domain = domain;
	}

private:
	IService::MetaInfo				m_meta;
	rttr::registration::class_<T>	m_class;
};

} // namespace engine::registration
