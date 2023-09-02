#include "LaunchEngine.hpp"
#include "Event.hpp"
#include "EventDispatcher.hpp"
#include "Logger.hpp"
#include "Window.hpp"
#include "Input.hpp"
#include "Scene.hpp"
#include "Assert.hpp"
#include "Renderer.hpp"
#include "Texture.hpp"
#include "MeshBuilder.hpp"
#include "Layer.hpp"
#include "Application.hpp"
#include "Types.hpp"
#include "Path.hpp"

extern RightEngine::GPU_API GGPU_API;

extern std::string G_ASSET_DIR;
extern std::string G_ENGINE_ASSET_DIR;
extern std::string G_CONFIG_DIR;
