#pragma once

#include <Engine/Config.hpp>

namespace engine
{

class ENGINE_API Engine
{
public:
	Engine(int argCount, char* argPtr[]);
	~Engine();

private:
};

} // namespace engine