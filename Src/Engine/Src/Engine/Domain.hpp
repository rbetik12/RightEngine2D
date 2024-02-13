#pragma once

#include <Engine/Config.hpp>
#include <Engine/EnumOps.hpp>

namespace engine
{

enum class ENGINE_API Domain : uint32_t
{
	NONE = 0x00,
	EDITOR = 0x01,
	CLIENT = 0x02,
	SERVER_UI = 0x04,
	SERVER_NO_UI = 0x08,

	SERVER = SERVER_UI | SERVER_NO_UI,

	UI = EDITOR | CLIENT | SERVER_UI,
	NO_EDITOR = CLIENT | SERVER,

	ALL = static_cast<uint32_t>(-1)
};
ENGINE_DEFINE_BITWISE_OPS(Domain);

inline std::string_view DomainToString(Domain domain)
{
	switch (domain)
	{
	case Domain::NONE: return "None";
	case Domain::EDITOR: return "Editor";
	case Domain::CLIENT: return "Client";
	case Domain::SERVER_UI: return "Server UI";
	case Domain::SERVER_NO_UI: return "Server No UI";
	case Domain::SERVER: return "Server";
	case Domain::UI: return "UI";
	case Domain::NO_EDITOR: return "No Editor";
	case Domain::ALL: return "All";
	default: 
		ENGINE_ASSERT(false);
		return "";
	}
}

}
