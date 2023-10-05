#pragma once

#include <Engine/Config.hpp>
#include <Engine/Domain.hpp>
#include <Core/Type.hpp>
#include <rttr/rttr_enable.h>

namespace engine
{

class ENGINE_API IService : public core::NonCopyable
{
	RTTR_ENABLE();
public:
	struct MetaInfo
	{
		Domain m_domain = Domain::ALL;
	};

	virtual ~IService() = default;

	virtual void Update(float dt) = 0;
	virtual void PostUpdate(float dt) = 0;
};

} // namespace engine