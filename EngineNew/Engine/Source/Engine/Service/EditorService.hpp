#pragma once

#include <Engine/Service/IService.hpp>

namespace engine
{

class ENGINE_API EditorService final : public IService
{
public:
	EditorService();
	virtual ~EditorService() override;

	virtual void	Update(float dt) override;
	virtual void	PostUpdate(float dt) override;

private:
};

} // namespace engine