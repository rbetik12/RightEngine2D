#pragma once
#include "Entity.hpp"
#include <Engine/Service/IService.hpp>

namespace editor
{
	class SelectionService : public engine::IService
	{
	public:
		virtual void OnUpdate(float dt) override;
		virtual void OnRegister() override;
		virtual ~SelectionService();

		void Entity(const std::shared_ptr<RightEngine::Entity>& entity);
		std::shared_ptr<RightEngine::Entity> Entity() const;

		using SelectCallback = std::function<void(std::shared_ptr<RightEngine::Entity>)>;
		using DeselectCallback = std::function<void()>;

		void SelectEntityCallback(SelectCallback&& cb)
		{
			m_selectEntityCallback = std::move(cb);
		}

		void DeselectEntityCallback(DeselectCallback&& cb)
		{
			m_deselectEntityCallback = std::move(cb);
		}

	private:
		std::weak_ptr<RightEngine::Entity> m_entity;
		SelectCallback m_selectEntityCallback;
		DeselectCallback m_deselectEntityCallback;
	};
}
