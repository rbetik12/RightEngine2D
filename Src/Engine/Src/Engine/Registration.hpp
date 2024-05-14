#pragma once

#include <Engine/Service/IService.hpp>
#include <Engine/Service/Resource/Loader.hpp>
#include <Engine/ECS/Component.hpp>
#include <Engine/ECS/System.hpp>
#include <Core/Hash.hpp>
#include <Core/RTTRIntegration.hpp>
#include <argparse/argparse.hpp>
#include <rttr/policy.h>

namespace engine::registration
{

constexpr uint64_t C_METADATA_KEY = core::hash::HashString("Metadata");
constexpr uint64_t C_PROJECT_SETTINGS_METADATA_KEY = core::hash::HashString("Project Settings Metadata");

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
    IService::MetaInfo                m_meta;
    rttr::registration::class_<T>    m_class;
};

template<typename T>
class ENGINE_API System
{
public:
    System(rttr::string_view name) : m_class(name)
    {
        static_assert(std::is_base_of_v<engine::ecs::System, T>, "T must be derived of engine::ecs::System");

        m_class.constructor<ecs::World*>();
    }

    ~System()
    {
        m_class(
            rttr::metadata(C_METADATA_KEY, std::move(m_meta))
        );
    }

    System& Domain(Domain domain)
    {
        m_meta.m_domain = domain;
        return *this;
    }

    template<typename TOther>
    System& UpdateAfter()
    {
        static_assert(std::is_base_of_v<engine::ecs::System, TOther>, "TOther must be derived of engine::ecs::System");
        static_assert(!std::is_same_v<T, TOther>, "Cycle in system update order");

        m_meta.m_updateAfter.emplace_back(rttr::type::get<TOther>());
        return *this;
    }

    template<typename TOther>
    System& UpdateBefore()
    {
        static_assert(std::is_base_of_v<System, TOther>, "TOther must be derived of engine::ecs::System");
        static_assert(!std::is_same_v<T, TOther>, "Cycle in system update order");

        m_meta.m_updateBefore.emplace_back(rttr::type::get<TOther>());
        return *this;
    }

private:
    engine::ecs::System::MetaInfo    m_meta;
    rttr::registration::class_<T>    m_class;
};

enum class CtorType : uint8_t
{
    AsObject,
    AsSharedPtr,
    AsRawPtr
};

template<typename T, CtorType type = CtorType::AsObject>
class ENGINE_API Class
{
public:
    explicit Class(rttr::string_view name) : m_class(name)
    {
        ENGINE_ASSERT_WITH_MESSAGE(!helpers::typeRegistered<T>(), fmt::format("Type '{}' was already registered!", rttr::type::get<T>().get_name()));

        if constexpr (type == CtorType::AsObject)
        {
            static_assert(std::is_copy_constructible_v<T>, "If you want to register non-copyable type, you can use pointer type");
            m_class.constructor()
            (
                rttr::policy::ctor::as_object
            );
        }
        else if constexpr (type == CtorType::AsRawPtr)
        {
            m_class.constructor()
            (
                rttr::policy::ctor::as_raw_ptr
            );
        }
        else if constexpr (type == CtorType::AsSharedPtr)
        {
            m_class.constructor()
            (
                rttr::policy::ctor::as_std_shared_ptr
            );
        }
    }

    template <typename PropType, typename ClassType>
    Class& Property(rttr::string_view name, PropType ClassType::* field)
    {
        static_assert(std::is_base_of_v<ClassType, T>);

        m_class.property(name, field);
        return *this;
    }

protected:
    rttr::registration::class_<T> m_class;
};


// TODO: Add domain support
template<typename T>
class ENGINE_API ResourceLoader : public Class<T, CtorType::AsRawPtr>
{
public:
    ResourceLoader(rttr::string_view name) : Class<T, CtorType::AsRawPtr>(name)
    {
        static_assert(std::is_base_of_v<Loader, T>, "Resource loader must be derived of Loader class");
    }
};

template<typename T>
class ENGINE_API ProjectSettings : public Class<T>
{
public:
    ProjectSettings(rttr::string_view name) : Class<T>(name) {}

    template <typename PropType, typename ClassType>
    ProjectSettings& property(rttr::string_view name, PropType ClassType::* field)
    {
        Class<T>::property(name, field);
        return *this;
    }

    ~ProjectSettings()
    {
        Class<T>::m_class(
            rttr::metadata(C_PROJECT_SETTINGS_METADATA_KEY, C_PROJECT_SETTINGS_METADATA_KEY)
        );
    }
};

template<typename T>
class ENGINE_API Component : public Class<T>
{
public:
    Component(ecs::Component::Type type, rttr::string_view name) : Class<T>(name) {}

    template <typename PropType, typename ClassType>
    Component& property(rttr::string_view name, PropType ClassType::* field)
    {
        Class<T>::property(name, field);
        return *this;
    }

    ~Component()
    {
        Class<T>::m_class(
            rttr::metadata(C_METADATA_KEY, std::move(m_meta))
        );
    }

private:
    engine::ecs::Component::MetaInfo m_meta;
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
    std::string_view    m_shortName;
    std::string_view    m_name;
    std::string_view    m_help;
    std::string_view    m_defaultValue;
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

} // engine::registration
