#pragma once

#if !defined(R_RTTR_INCLUDE_GUARD)
#    if defined(RTTR_CORE_PREREQUISITES_H_)
#        error "Some RTTR header was included before <Core/RTTRIntegration.hpp>"
#    endif
#    define R_RTTR_INCLUDE_GUARD
#endif

#include <Core/Config.hpp>
#include <Core/EASTLIntergration.hpp>
#include <Core/Hash.hpp>
#include <Core/NatvisHolder.hpp>

#include <rttr/registration>
#include <rttr/registration_friend>
#include <rttr/rttr_enable.h>
#include <rttr/type>
#include <rttr/variant.h>
#include <fmt/format.h>

namespace eastl
{

template <>
struct hash<rttr::type>
{
    inline size_t operator()(rttr::type value) const
    {
        return hash<rttr::type::type_id>()(value.get_id());
    }
};

template <>
struct hash<rttr::property>
{
    inline size_t operator()(rttr::property value) const
    {
        const auto h1 = hash<rttr::type>()(value.get_declaring_type());
        const auto h2 = hash<std::string_view>()(value.get_name().data());

        return h1 ^ (h2 << 1);
    }
};

} // eastl

namespace rttr
{

template <typename T, typename A>
struct sequential_container_mapper<eastl::vector<
        T, A>> : detail::sequential_container_base_dynamic_direct_access<eastl::vector<T, A>>
{
};

template <typename T, size_t N, bool O>
struct sequential_container_mapper<eastl::fixed_vector<
        T, N, O>> : detail::sequential_container_base_dynamic_direct_access<eastl::fixed_vector<T, N, O>>
{
};

template <typename T>
struct sequential_container_mapper<eastl::list<
        T>> : detail::sequential_container_base_dynamic_itr_access<eastl::list<T>>
{
};

template <typename T>
struct sequential_container_mapper<eastl::deque<
        T>> : detail::sequential_container_base_dynamic_direct_access<eastl::deque<T>>
{
};

template <typename T, size_t N>
struct sequential_container_mapper<eastl::array<
        T, N>> : detail::sequential_container_base_static<eastl::array<T, N>>
{
};

// Associative containers
namespace detail
{

template <typename T>
struct eastl_iterator_wrapper_base
{
    using Itr = T;

    static const Itr& get_iterator(const iterator_data& data)
    {
        return *reinterpret_cast<Itr* const&>(data);
    }

    static Itr& get_iterator(iterator_data& data)
    {
        return *reinterpret_cast<Itr*&>(data);
    }

    static void create(iterator_data& itr_tgt, const Itr& itr_src)
    {
        reinterpret_cast<Itr*&>(itr_tgt) = new Itr(itr_src);
    }

    static void create(iterator_data& itr_tgt, const iterator_data& itr_src)
    {
        reinterpret_cast<Itr*&>(itr_tgt) = new Itr(*reinterpret_cast<Itr* const&>(itr_src));
    }

    static void destroy(iterator_data& itr)
    {
        auto& it = get_iterator(itr);
        delete&it;
    }

    static void advance(iterator_data& itr, std::ptrdiff_t idx)
    {
        eastl::advance(get_iterator(itr), idx);
    }

    static bool equal(const iterator_data& lhs_itr, const iterator_data& rhs_itr) RTTR_NOEXCEPT
    {
        return (get_iterator(lhs_itr) == get_iterator(rhs_itr));
    }
};

template <typename Value, bool bConst, bool bCacheHashCode>
struct iterator_wrapper_base<eastl::hashtable_iterator<
        Value, bConst, bCacheHashCode>> : eastl_iterator_wrapper_base<eastl::hashtable_iterator<
        Value, bConst, bCacheHashCode>>
{
};

template <typename T, typename Pointer, typename Reference>
struct iterator_wrapper_base<eastl::rbtree_iterator<
        T, Pointer, Reference>> : eastl_iterator_wrapper_base<eastl::rbtree_iterator<T, Pointer, Reference>>
{
};

template <typename T>
struct eastl_associative_container_base
{
    using container_t = T;
    using key_t = typename container_t::key_type;
    using value_t = typename container_t::mapped_type;
    using itr_t = typename container_t::iterator;
    using const_itr_t = typename container_t::const_iterator;
    using size_t = typename container_t::size_type;

    static const key_t& get_key(const const_itr_t& itr)
    {
        return itr->first;
    }

    static value_t& get_value(itr_t& itr)
    {
        return itr->second;
    }

    static const value_t& get_value(const const_itr_t& itr)
    {
        return itr->second;
    }

    static itr_t begin(container_t& container)
    {
        return container.begin();
    }

    static const_itr_t begin(const container_t& container)
    {
        return container.begin();
    }

    static itr_t end(container_t& container)
    {
        return container.end();
    }

    static const_itr_t end(const container_t& container)
    {
        return container.end();
    }

    static itr_t find(container_t& container, key_t& key)
    {
        return container.find(key);
    }

    static const_itr_t find(const container_t& container, key_t& key)
    {
        return container.find(key);
    }

    static std::pair<itr_t, itr_t> equal_range(container_t& container, key_t& key)
    {
        auto ret = container.equal_range(key);
        return std::make_pair(ret.first, ret.second);
    }

    static std::pair<const_itr_t, const_itr_t> equal_range(const container_t& container, key_t& key)
    {
        auto ret = container.equal_range(key);
        return std::make_pair(ret.first, ret.second);
    }

    static void clear(container_t& container)
    {
        container.clear();
    }

    static bool is_empty(const container_t& container)
    {
        return container.empty();
    }

    static size_t get_size(const container_t& container)
    {
        return container.size();
    }

    static size_t erase(container_t& container, key_t& key)
    {
        return container.erase(key);
    }

    template <typename K, typename V>
    static std::pair<itr_t, bool> insert_key_value(container_t& container, K&& key, V&& value)
    {
        if (auto it = container.find(key); it != container.end())
        {
            it->second = std::forward<V>(value);
            return std::make_pair(it, true);
        }

        auto ret = container.emplace(std::forward<K>(key), std::forward<V>(value));
        return std::make_pair(ret.first, ret.second);
    }
};

template <typename T>
struct eastl_associative_container_key_base
{
    using container_t = T;
    using key_t = typename T::key_type;
    using value_t = typename associative_container_value_t<T>::type;
    using itr_t = typename T::iterator;
    using const_itr_t = typename T::const_iterator;
    using size_t = typename T::size_type;

    static const key_t& get_key(const const_itr_t& itr)
    {
        return *itr;
    }

    static itr_t begin(container_t& container)
    {
        return container.begin();
    }

    static const_itr_t begin(const container_t& container)
    {
        return container.begin();
    }

    static itr_t end(container_t& container)
    {
        return container.end();
    }

    static const_itr_t end(const container_t& container)
    {
        return container.end();
    }

    static itr_t find(container_t& container, const key_t& key)
    {
        return container.find(key);
    }

    static const_itr_t find(const container_t& container, const key_t& key)
    {
        return container.find(key);
    }

    static std::pair<itr_t, itr_t> equal_range(container_t& container, const key_t& key)
    {
        auto ret = container.equal_range(key);
        return std::make_pair(ret.first, ret.second);
    }

    static std::pair<const_itr_t, const_itr_t> equal_range(const container_t& container, const key_t& key)
    {
        auto ret = container.equal_range(key);
        return std::make_pair(ret.first, ret.second);
    }

    static void clear(container_t& container)
    {
        container.clear();
    }

    static bool is_empty(const container_t& container)
    {
        return container.empty();
    }

    static size_t get_size(const container_t& container)
    {
        return container.size();
    }

    static size_t erase(container_t& container, const key_t& key)
    {
        return container.erase(key);
    }

    template <typename K>
    static std::pair<itr_t, bool> insert_key(container_t& container, K&& key)
    {
        auto ret = container.insert(std::forward<K>(key));
        return std::make_pair(ret.first, ret.second);
    }
};

} // detail

template <typename K, typename T>
struct associative_container_mapper<eastl::unordered_map<
        K, T>> : detail::eastl_associative_container_base<eastl::unordered_map<K, T>>
{
};

template <typename K, typename T>
struct associative_container_mapper<eastl::map<K, T>> : detail::eastl_associative_container_base<eastl::map<K, T>>
{
};

template <typename K, typename T, typename Compare>
struct associative_container_mapper<eastl::vector_map<
        K, T, Compare>> : detail::eastl_associative_container_base<eastl::vector_map<K, T, Compare>>
{
};

template <typename T>
struct associative_container_mapper<eastl::set<T>> : detail::eastl_associative_container_key_base<eastl::set<T>>
{
};

template <typename T, typename Compare>
struct associative_container_mapper<eastl::set<
        T, Compare>> : detail::eastl_associative_container_key_base<eastl::set<T, Compare>>
{
};

template <typename K>
struct associative_container_mapper<eastl::vector_set<
        K>> : detail::eastl_associative_container_key_base<eastl::vector_set<K>>
{
};

} // rttr

namespace rttr_natvis
{

class CORE_API RttrNatvisFactories
{
public:
    static RttrNatvisFactories& Instance();

    class IRTTRNatvis : public core::INatvisHolder
    {
    public:
        virtual core::INatvisHolder* Value(const rttr::variant& v) = 0;
    };

    template<typename T>
    class RTTRNatvisHolder : public IRTTRNatvis
    {
        explicit RTTRNatvisHolder(const T* p) : m_p(p) {}

    public:
        RTTRNatvisHolder() = default;

        core::INatvisHolder* Value(const rttr::variant& v) override
        {
            static auto rttrType = rttr::type::get<T>();

            const T* p = nullptr;

            if (v.get_type().is_pointer())
            {
                p = v.get_value_unsafe<T*>();
            }
            else if (v.get_type().is_wrapper())
            {
                p = &v.get_wrapped_value_unsafe<T>();
            }
            else
            {
                p = &v.get_value_unsafe<T>();
            }

            return new RTTRNatvisHolder(p);
        }

    private:
        const T* m_p = nullptr;
    };

    template<typename T>
    void RegisterType()
    {
        static_assert(!std::is_const_v<T> && !std::is_pointer_v<T>);

        auto factory = std::make_shared<RTTRNatvisHolder<T>>();

        RegisterSingle<T>(factory);
    }

    void Cleanup();

private:
    template<typename T>
    void RegisterSingle(std::shared_ptr<IRTTRNatvis> factory)
    {
        m_fns[rttr::type::get<T>()] = factory;
    }

    core::INatvisHolder* GetNatvis(const void* var);

private:
    eastl::unordered_map<rttr::type, std::shared_ptr<IRTTRNatvis>> m_fns;
};

} // rttr_natvis

#define RTTR_NATVIS_REGISTER(T) rttr_natvis::RttrNatvisFactories::Instance().RegisterType<T>()

namespace core
{

template<typename T>
class RTTRObject
{
public:
    explicit RTTRObject(std::string_view name) : m_class(validate(name)) { }

    ~RTTRObject()
    {
        RTTR_NATVIS_REGISTER(T);
    }

private:
    static std::string_view validate(std::string_view name)
    {
        const auto typeByName = rttr::type::get_by_name(name);

        CORE_ASSERT_WITH_MESSAGE((!typeByName.is_valid() 
            || !rttr::type::get<T>().is_valid() 
            || (rttr::type::get<T>().get_id() == typeByName.get_id())), 
            "Looks like you're trying to register type twice or with the same name.");

        return name;
    }

protected:
    rttr::registration::class_<T> m_class;
};

} // core
