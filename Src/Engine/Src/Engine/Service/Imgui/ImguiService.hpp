#pragma once

#include <Engine/Config.hpp>
#include <Engine/Service/IService.hpp>
#include <RHI/IImguiProvider.hpp>

namespace engine
{

class ENGINE_API ImguiService : public IService
{
    RTTR_ENABLE(IService);
public:
    ImguiService();
    virtual ~ImguiService() override;

    virtual void    Update(float dt) override;
    virtual void    PostUpdate(float dt) override;

    void            Image(const std::shared_ptr<rhi::Texture>& texture, const ImVec2& size, const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1, 1));
    void            RemoveImage(const std::shared_ptr<rhi::Texture>& texture);

private:
    std::shared_ptr<rhi::imgui::IImguiProvider> m_imguiProvider;
};

} // engine