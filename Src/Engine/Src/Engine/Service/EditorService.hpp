#pragma once

#include <Engine/Service/IService.hpp>
#include <RHI/ShaderCompiler.hpp>
#include <entt/entity/entity.hpp>

namespace engine
{

namespace editor
{
constexpr entt::entity C_INVALID_ENTITY = (entt::entity) std::numeric_limits<uint32_t>::max();
} // editor

class ENGINE_API EditorService final : public Service<EditorService>
{
public:
    struct Impl;

    EditorService();
    virtual ~EditorService() override;

    virtual void    Update(float dt) override;
    virtual void    PostUpdate(float dt) override;

    void            SelectedEntity(entt::entity e);
    entt::entity    SelectedEntity();

    void            Initialize();

    glm::ivec2      ViewportSize() const;
    bool            IsViewportHovered() const;

private:
    std::unique_ptr<Impl> m_impl;
};

} // namespace engine