#include "Engine.hpp"

namespace gust
{
	Engine Engine::engine = {};

	void Engine::startup(const std::string& name, uint32_t width, uint32_t height)
	{
		input.startup();
		graphics.startup(name, width, height);
		resourceManager.startup(&graphics, 20, 20, 10, 10);
		renderer.startup(&graphics);
	}

	void Engine::simulate()
	{
		while (!input.isClosing())
		{
			input.pollEvents();
		}
	}

	void Engine::shutdown()
	{
		renderer.shutdown();
		resourceManager.shutdown();
		graphics.shutdown();
		input.shutdown();
	}
}