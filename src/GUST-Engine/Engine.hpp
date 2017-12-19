#pragma once

/**
 * @file Engine.hpp
 * @brief Engine header file.
 * @author Connor J. Bramham (ReeCocho)
 */

/** Includes. */
#include "Graphics\Graphics.hpp"
#include "Graphics\Renderer.hpp"
#include "Core\Input.hpp"
#include "Core\ResourceManager.hpp"
#include "Core\Threading.hpp"
#include "ECS\Scene.hpp"
#include "Utilities\Clock.hpp"

namespace gust
{
	/**
	 * @class Engine
	 * @brief Game engine singleton.
	 */
	class Engine
	{
	public:

		/**
		 * @brief Default constructor.
		 */
		Engine() = default;

		/**
		 * @brief Default destructor.
		 */
		~Engine() = default;

		/**
		 * @brief Startup the engine.
		 * @param Window name.
		 * @param Window width.
		 * @param Window height.
		 * @note This is called internally. Do not use.
		 */	
		void startup(const std::string& name, uint32_t width, uint32_t height);

		/**
		 * @brief Simulate the engine until completion.
		 * @note This is called internally. Do not use.
		 */
		void simulate();

		/**
		 * @brief Shutdown the engine.
		 * @note This is called internally. Do not use.
		 */
		void shutdown();

		/**
		 * @brief Get engine singleton.
		 * @return Engine singleton.
		 */
		static inline Engine& get()
		{
			return engine;
		}

		/**
		 * @brief Get the frame rate.
		 * @return Frame rate.
		 */
		inline uint32_t getFrameRate() const
		{
			return m_frameRate;
		}


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

	private:

		/** Game clock. */
		Clock m_clock = {};

		/** Frame counter. */
		uint64_t m_frameCounter = 0;

		/** Time elapsed since last measuring the framerate. */
		float m_frameRateTimer = 0;

		/** Frame rate. */
		uint32_t m_frameRate = 0;

		/** Rendering thread. */
		std::unique_ptr<SimulationThread> m_renderingThread;

		/** Singleton. */
		static Engine engine;
	};
}