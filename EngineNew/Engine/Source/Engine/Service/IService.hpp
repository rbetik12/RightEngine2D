#pragma once

#include <Engine/Config.hpp>
#include <Core/Type.hpp>

namespace engine
{

class ENGINE_API IService : public core::NonCopyable
{
public:
	virtual ~IService() = default;

	virtual void Init() = 0;
	virtual void Destroy() = 0;

	virtual void Update(float dt) = 0;
	virtual void PostUpdate(float dt) = 0;
};

} // namespace engine