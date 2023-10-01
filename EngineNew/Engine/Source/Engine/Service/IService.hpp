#pragma once

#include <Engine/Config.hpp>

namespace engine
{
	class ENGINE_API IService
	{
	public:
		virtual void Init() = 0;
		virtual void Update() = 0;
		virtual void Destroy() = 0;
	};
}