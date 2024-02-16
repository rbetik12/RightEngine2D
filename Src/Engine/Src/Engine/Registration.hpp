#pragma once

#include <Engine/Service/IService.hpp>
#include <Core/Hash.hpp>
#include <argparse/argparse.hpp>
#include <rttr/registration>

namespace engine::registration
{

constexpr uint64_t C_METADATA_KEY = core::hash::HashString("Metadata");

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
		m_class(
			rttr::metadata(C_METADATA_KEY, std::move(m_meta))
		);
	}

	Service& Domain(Domain domain)
	{
		m_meta.m_domain = domain;
		return *this;
	}

	template<typename TOther>
	Service& UpdateAfter()
	{
		static_assert(std::is_base_of_v<IService, TOther>, "TOther must be derived of engine::IService");
		static_assert(!std::is_same_v<T, TOther>, "Cycle in service update order");

		m_meta.m_updateAfter.emplace_back(rttr::type::get<TOther>());
		return *this;
	}

	template<typename TOther>
	Service& UpdateBefore()
	{
		static_assert(std::is_base_of_v<IService, TOther>, "TOther must be derived of engine::IService");
		static_assert(!std::is_same_v<T, TOther>, "Cycle in service update order");

		m_meta.m_updateBefore.emplace_back(rttr::type::get<TOther>());
		return *this;
	}

	template<typename TOther>
	Service& PostUpdateAfter()
	{
		static_assert(std::is_base_of_v<IService, TOther>, "TOther must be derived of engine::IService");
		static_assert(!std::is_same_v<T, TOther>, "Cycle in service post update order");

		m_meta.m_postUpdateAfter.emplace_back(rttr::type::get<TOther>());
		return *this;
	}

	template<typename TOther>
	Service& PostUpdateBefore()
	{
		static_assert(std::is_base_of_v<IService, TOther>, "TOther must be derived of engine::IService");
		static_assert(!std::is_same_v<T, TOther>, "Cycle in service post update order");

		m_meta.m_postUpdateBefore.emplace_back(rttr::type::get<TOther>());
		return *this;
	}

private:
	IService::MetaInfo				m_meta;
	rttr::registration::class_<T>	m_class;
};

template<typename T>
class ENGINE_API Class
{
public:
	Class(rttr::string_view name) : m_class(name)
	{
		m_class.constructor();
	}

private:
	rttr::registration::class_<T> m_class;
};

class ENGINE_API CommandLineArg
{
public:
	CommandLineArg(std::string_view shortName, std::string_view name) : m_shortName(shortName),
																		m_name(name)
	{}

	CommandLineArg& DefaultValue(std::string_view defaultValue)
	{
		m_defaultValue = defaultValue;
		return *this;
	}

	CommandLineArg& Help(std::string_view helpMessage)
	{
		m_help = helpMessage;
		return *this;
	}

	std::string_view ShortName() const
	{
		return m_shortName;
	}

	std::string_view Name() const
	{
		return m_name;
	}

	std::string_view Help() const
	{
		return m_help;
	}

	std::string_view DefaultValue() const
	{
		return m_defaultValue;
	}

private:
	std::string_view	m_shortName;
	std::string_view	m_name;
	std::string_view	m_help;
	std::string_view	m_defaultValue;
};

class ENGINE_API CommandLineArgs
{
public:
	CommandLineArgs()
	{
		if (!m_parser)
		{
			m_parser = new argparse::ArgumentParser("Right Engine");
		}
	}

	static void Parse(int argc, char* argv[])
	{
		m_parser->parse_args(argc, argv);
	}

	CommandLineArgs& Argument(CommandLineArg& arg)
	{
		auto& parserArg = m_parser->add_argument(arg.ShortName(), arg.Name());

		if (!arg.DefaultValue().empty())
		{
			parserArg.default_value(std::string{ arg.DefaultValue() });
		}

		if (!arg.Help().empty())
		{
			parserArg.help(std::string{ arg.Help() });
		}

		return *this;
	}

	static std::string_view Get(std::string_view name)
	{
		return m_parser->get<std::string>(name);
	}

private:
	inline static argparse::ArgumentParser* m_parser = nullptr;
};

namespace helpers
{

template<typename T>
inline bool typeRegistered()
{
	return rttr::type::get<T>().get_constructor().is_valid();
}

inline bool typeRegistered(rttr::type type)
{
	return type.get_constructor().is_valid();
}

} // helpers

} // engine::registration
