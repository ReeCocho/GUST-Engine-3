#include "Engine.hpp"

namespace gust
{
	Engine Engine::engine = {};

	void Engine::startup(const std::string& name, uint32_t width, uint32_t height)
	{
		// Start modules
		input.startup();
		graphics.startup(name, width, height);
		resourceManager.startup(&graphics, 20, 20, 10, 10);
		renderer.startup(&graphics, 4);

		// Start threads
		m_renderingThread = std::make_unique<SimulationThread>([this]() { renderer.render(); });
	}

	void Engine::simulate()
	{
		while (!input.isClosing())
		{
			// Gather input
			input.pollEvents();

			// Wait for threads to finish processing
			m_renderingThread->wait();

			// Run game code
			// ...

			// Start threads for rendering and physics
			m_renderingThread->start();
		}
	}

	void Engine::shutdown()
	{
		// Stop threads
		m_renderingThread = nullptr;

		// Shutdown modules
		renderer.shutdown();
		resourceManager.shutdown();
		graphics.shutdown();
		input.shutdown();
	}
}