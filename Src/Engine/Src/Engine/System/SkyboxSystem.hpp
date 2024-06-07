#pragma once

#include <Engine/ECS/System.hpp>
#include <Engine/Service/Resource/TextureResource.hpp>
#include <RHI/Buffer.hpp>

namespace engine
{

struct ENGINE_API SkyboxComponent : public ecs::Component
{
    RPtr<MaterialResource>  m_skyboxMaterial;
    RPtr<rhi::Texture>      m_irradianceTexture;
    RPtr<rhi::Texture>      m_prefilterTexture;
    RPtr<rhi::Texture>      m_brdfTexture;
};

class ENGINE_API SkyboxSystem : public ecs::System
{
    RTTR_ENABLE(System)
public:
    SkyboxSystem(ecs::World* world);
    virtual ~SkyboxSystem() = default;

    virtual void Update(float dt) override;

private:
    std::shared_ptr<rhi::Buffer> m_skyboxVB;
};

} // engine