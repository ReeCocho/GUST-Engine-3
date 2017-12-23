#include <iostream>
#include <tuple>
#include "Engine.hpp"
#include "Colliders.hpp"

namespace
{
	/** Game clock. */
	gust::Clock gameClock = {};

	/** Frame counter. */
	uint64_t frameCounter = 0;

	/** Time elapsed since last measuring the framerate. */
	float frameRateTimer = 0;

	/** Time elapsed since last performing a physics step. */
	float physicsTimer = 0;

	/** Frame rate. */
	uint32_t frameRate = 0;

	/** Rendering thread. */
	std::unique_ptr<gust::SimulationThread> renderingThread;

	/** Physics thread. */
	std::unique_ptr<gust::SimulationThread> physicsThread;

	/** Collision callbacks. */
	std::vector<std::tuple<gust::Handle<gust::Collider>, std::function<void(gust::CollisionData)>>> collisionCallbacks = {};
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
		renderingThread = std::make_unique<SimulationThread>([]() { renderer.render(); });
		physicsThread = std::make_unique<SimulationThread>([]() 
		{ 
			physics.step(GUST_PHYSICS_STEP_RATE);
		});
	}

	void simulate()
	{
		while (!input.isClosing())
		{
			// Get delta time
			float deltaTime = gameClock.getDeltaTime();

			// Increment physics timer
			physicsTimer += deltaTime;

			// Do framerate stuff
			++frameCounter;
			frameRateTimer += deltaTime;

			if (frameRateTimer >= 1.0f)
			{
				frameRate = static_cast<uint32_t>(frameCounter);
				frameCounter = 0;
				frameRateTimer = 0;

				std::cout << frameRate << '\n';
			}

			// Gather input
			input.pollEvents();

			// Wait for threads to finish processing
			renderingThread->wait();
			physicsThread->wait();

			// Run game code
			{
				// Collision callbacks
				PhysicsCollisionData data = {};
				while (physics.pollPhysicsCollisionData(data))
				{
					// Loop over every collision callback
					for (size_t i = 0; i < collisionCallbacks.size(); ++i)
					{
						auto collider = std::get<0>(collisionCallbacks[i]);
						auto c = collider.get();

						std::cout << collider->getCollisionShape() << ' ' << data.touched->getCollisionShape() << '\n';

						// If the collision involes the collider in the callback...
						if (collider->getCollisionShape() == data.touched->getCollisionShape())
						{
							CollisionData callbackData = {};
							callbackData.normal = data.normal;
							callbackData.point = data.point;
							callbackData.touched = collider;
							callbackData.touching = Collider::getColliderByShape(data.touching->getCollisionShape());

							std::get<1>(collisionCallbacks[i])(callbackData);
						}
					}
				}

				// Scene tick
				scene.tick(deltaTime);
			}

			// Start threads for rendering and physics
			renderingThread->start();

			if (physicsTimer >= GUST_PHYSICS_STEP_RATE)
			{
				physicsThread->start();
				physicsTimer = 0;
			}
		}
	}

	void shutdown()
	{
		// Stop threads
		physicsThread = nullptr;
		renderingThread = nullptr;

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
		return frameRate;
	}

	void registerCollisionCallback(Handle<Collider> collider, std::function<void(CollisionData)> callback)
	{
		collisionCallbacks.push_back(std::make_tuple(collider, callback));
	}
}