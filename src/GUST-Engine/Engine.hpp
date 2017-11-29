#pragma once

/**
 * @file Engine.hpp
 * @brief Engine header file.
 * @author Connor J. Bramham (ReeCocho)
 */

/** Includes. */
#include "Graphics\Graphics.hpp"
#include "Core\Input.hpp"
#include "Core\ResourceManager.hpp"

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


		/** Graphics context. */
		Graphics graphics = {};

		/** Input manager. */
		Input input = {};

		/** Resource manager. */
		ResourceManager resourceManager = {};

	private:

		/** Singleton. */
		static Engine engine;
	};
}