#include "EntryPoint.hpp"
#include "EditorLayer.hpp"

static std::shared_ptr<editor::EditorLayer> layer;

void GameApplication::OnStart()
{
    layer = std::make_shared<editor::EditorLayer>();
    RightEngine::Application::Get().PushLayer(layer);
}

void GameApplication::OnUpdate()
{
}

void GameApplication::OnDestroy()
{
}

std::string G_ASSET_DIR = ASSETS_DIR;
std::string G_CONFIG_DIR = CONFIG_DIR;
