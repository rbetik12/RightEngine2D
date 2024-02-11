#include <Engine/Engine.hpp>
#include <Engine/Domain.hpp>
#include <Engine/Service/WindowService.hpp>
#include <Engine/Service/ThreadService.hpp>
#include <Engine/Service/EditorService.hpp>
#include <Engine/Service/Render/RenderService.hpp>

RTTR_REGISTRATION
{
using namespace engine::registration;
using namespace rttr;

CommandLineArgs()
			.Argument(
				CommandLineArg("-m", "--mode")
				.Help("Engine launch mode - client, server, server_no_ui or editor")
				.DefaultValue("editor")
			);


registration::enumeration<engine::Domain>("engine::Domain")(
		value("none", engine::Domain::NONE),
		value("editor", engine::Domain::ALL),
		value("client", engine::Domain::CLIENT),
		value("server", engine::Domain::SERVER_UI),
		value("server_no_ui", engine::Domain::SERVER_NO_UI)
	);
}

namespace engine
{

Engine::Engine(int argCount, char* argPtr[])
{
	m_timer.Restart();
	s_instance = this;

	ParseCfg(argCount, argPtr);

	core::log::info("Engine domain: {}", DomainToString(m_config.m_domain));

	m_serviceManager = std::make_unique<ServiceManager>(m_config.m_domain);

	m_serviceManager->RegisterService<ThreadService>();
	m_serviceManager->RegisterService<WindowService>();
	m_serviceManager->RegisterService<RenderService>();
	m_serviceManager->RegisterService<EditorService>();

	m_running = true;

	core::log::info("Engine was initialized successfully for {}s", m_timer.TimeInSeconds());
}

Engine::~Engine()
{
	m_serviceManager->Destroy();
	core::log::info("Engine was destroyed successfully");
}

int Engine::Run()
{
	core::log::info("Starting engine loop");

	m_timer.Restart();
	while (m_running)
	{
		Update();
	}

	core::log::info("Stopped engine loop");
	return 0;
}

void Engine::Stop()
{
	core::log::info("Engine loop stop requested");
	m_running = false;
}

void Engine::Update()
{
	m_timer.Stop();

	const float dt = std::max<float>(0.0001f, m_timer.TimeInMilliseconds() * 0.001f);
	m_timer.Start();

	m_serviceManager->Update(dt);
	m_serviceManager->PostUpdate(dt);
}

void Engine::ParseCfg(int argCount, char* argPtr[])
{
	registration::CommandLineArgs::Parse(argCount, argPtr);

	const auto launchMode = registration::CommandLineArgs::Get("--mode");
	m_config.m_domain = rttr::type::get_by_name("engine::Domain")
								.get_enumeration()
								.name_to_value(launchMode.data())
								.get_value<Domain>();
}

} // namespace engine

