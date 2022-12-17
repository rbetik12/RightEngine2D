#pragma once

#include "Core.hpp"
#include "Texture.hpp"

class SandboxLayer : public RightEngine::Layer
{
public:
    SandboxLayer(): Layer("Sandbox") {}

    virtual void OnAttach() override;
    virtual void OnUpdate(float ts) override;
    virtual void OnImGuiRender();
    bool OnEvent(const Event& event);
    
private:
    std::shared_ptr<RightEngine::Scene> scene;
};
