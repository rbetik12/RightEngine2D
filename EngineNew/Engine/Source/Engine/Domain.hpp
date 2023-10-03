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
	NO_EDITOR = CLIENT | SERVER


};
ENGINE_DEFINE_BITWISE_OPS(Domain);

}
