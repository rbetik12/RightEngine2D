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

private:
	IService::MetaInfo				m_meta;
	rttr::registration::class_<T>	m_class;
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

} // namespace engine::registration
