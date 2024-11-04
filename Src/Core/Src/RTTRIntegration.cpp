#include <Core/RTTRIntegration.hpp>

RTTR_REGISTRATION
{
	RTTR_NATVIS_REGISTER(bool);
	RTTR_NATVIS_REGISTER(float);
	RTTR_NATVIS_REGISTER(double);
	RTTR_NATVIS_REGISTER(char);
	RTTR_NATVIS_REGISTER(short);
	RTTR_NATVIS_REGISTER(int);
	RTTR_NATVIS_REGISTER(int64_t);
	RTTR_NATVIS_REGISTER(unsigned char);
	RTTR_NATVIS_REGISTER(unsigned short);
	RTTR_NATVIS_REGISTER(unsigned int);
	RTTR_NATVIS_REGISTER(uint64_t);
	RTTR_NATVIS_REGISTER(std::string);
	RTTR_NATVIS_REGISTER(std::string_view);

	rttr::type::register_converter_func(
		[](const rttr::type& t, bool& ok) -> std::string
		{
			ok = t.is_valid();
			return std::string(t.get_name());
		}
	);

	rttr::type::register_converter_func(
		[](const std::string& t, bool& ok) -> rttr::type
		{
			auto type = rttr::type::get_by_name(t);
			ok = type.is_valid();
			return type;
		}
	);

	rttr::type::register_converter_func(
		[](std::string_view t, bool& ok) -> rttr::type
		{
			auto type = rttr::type::get_by_name(t);
			ok = type.is_valid();
			return type;
		}
	);

	RTTR_NATVIS_REGISTER(rttr::type);
}

namespace rttr_natvis
{

RttrNatvisFactories& RttrNatvisFactories::Instance()
{
	static RttrNatvisFactories s_inst;
	return s_inst;
}

core::INatvisHolder* RttrNatvisFactories::GetNatvis(const void* var)
{
	auto* v = reinterpret_cast<const rttr::variant*>(var);

	if (v && v->is_valid())
	{
		auto type = v->get_type();
		if (auto it = m_fns.find(type.is_wrapper() ? type.get_wrapped_type().get_raw_type() : type.get_raw_type()); it != m_fns.end())
		{
			return it->second->Value(*v);
		}
		return new core::NatvisHolder(type.get_name());
	}

	static const char* C_INVALID = "Invalid variant";

	return new core::NatvisHolder(C_INVALID);
}

void RttrNatvisFactories::Cleanup()
{
	m_fns.clear();
}

} // rttr_natvis