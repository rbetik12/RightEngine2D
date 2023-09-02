#pragma once

#include <Core/Type.hpp>

namespace engine
{
	class IService : public core::NonCopyable
	{
	public:
		virtual void OnUpdate(float dt) = 0;
		virtual void OnRegister() = 0;
		virtual ~IService() = default;
	};
}
