#pragma once

/**
 * @file Scene.hpp
 * @brief Scene header file.
 * @author Connor J. Bramham (ReeCocho)
 */

/** Includes. */
#include <vector>
#include <memory>
#include "System.hpp"

namespace gust
{
	/**
	 * @class Scene
	 * @brief Manages systems and entities.
	 */
	class Scene
	{
	public:

		/**
		 * @brief Default constructor.
		 */
		Scene() = default;

		/**
		 * @brief Default destructor.
		 */
		~Scene() = default;

		/**
		 * @brief Initialize scene.
		 * @note Used internally. Do not call.
		 */
		void startup();

		/**
		 * @brief Shut down scene.
		 * @note Used internally. Do not call.
		 */
		void shutdown();

		/**
		 * @brief Perform a tick in the scene.
		 * @param Delta time.
		 */
		void tick(float deltaTime);

	private:

		/** Vector of systems. */
		std::vector<std::unique_ptr<SystemBase>> m_systems;
	};
}