#include <iostream>
#include "Engine.hpp"

namespace
{
	/** Game clock. */
	gust::Clock m_clock = {};

	/** Frame counter. */
	uint64_t m_frameCounter = 0;

	/** Time elapsed since last measuring the framerate. */
	float m_frameRateTimer = 0;

	/** Time elapsed since last performing a physics step. */
	float m_physicsTimer = 0;

	/** Frame rate. */
	uint32_t m_frameRate = 0;

	/** Rendering thread. */
	std::unique_ptr<gust::SimulationThread> m_renderingThread;

	/** Physics thread. */
	std::unique_ptr<gust::SimulationThread> m_physicsThread;
}

namespace gust
{
	/** Graphics context. */
	Graphics graphics = {};

	/** Input manager. */
	Input input = {};

	/** Resource manager. */
	ResourceManager resourceManager = {};

	/** Rendering engine. */
	Renderer renderer = {};

	/** Primary scene. */
	Scene scene = {};

	/** Physics engine. */
	Physics physics = {};



	void startup(const std::string& name, uint32_t width, uint32_t height)
	{
		// Start modules
		input.startup();
		graphics.startup(name, width, height);
		resourceManager.startup(&graphics, &renderer, 20, 20, 10, 10);
		renderer.startup(&graphics, 4);
		scene.startup();
		physics.startup({ 0, -9.82f, 0 });

		// Start threads
		m_renderingThread = std::make_unique<SimulationThread>([]() { renderer.render(); });
		m_physicsThread = std::make_unique<SimulationThread>([]() { physics.step(GUST_PHYSICS_STEP_RATE); });
	}

	void simulate()
	{
		while (!input.isClosing())
		{
			// Get delta time
			float deltaTime = m_clock.getDeltaTime();

			// Increment physics timer
			m_physicsTimer += deltaTime;

			// Do framerate stuff
			++m_frameCounter;
			m_frameRateTimer += deltaTime;

			if (m_frameRateTimer >= 1.0f)
			{
				m_frameRate = static_cast<uint32_t>(m_frameCounter);
				m_frameCounter = 0;
				m_frameRateTimer = 0;

				std::cout << m_frameRate << '\n';
			}

			// Gather input
			input.pollEvents();

			// Wait for threads to finish processing
			m_renderingThread->wait();
			m_physicsThread->wait();

			// Run game code
			scene.tick(deltaTime);

			// Start threads for rendering and physics
			m_renderingThread->start();

			if (m_physicsTimer >= GUST_PHYSICS_STEP_RATE)
			{
				m_physicsThread->start();
				m_physicsTimer = 0;
			}
		}
	}

	void shutdown()
	{
		// Stop threads
		m_physicsThread = nullptr;
		m_renderingThread = nullptr;

		// Shutdown modules
		scene.shutdown();
		physics.shutdown();
		renderer.shutdown();
		resourceManager.shutdown();
		graphics.shutdown();
		input.shutdown();
	}

	uint32_t getFrameRate()
	{
		return m_frameRate;
	}
}