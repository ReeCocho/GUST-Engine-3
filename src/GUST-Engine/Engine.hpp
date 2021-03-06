#pragma once

/**
 * @file Engine.hpp
 * @brief Engine header file.
 * @author Connor J. Bramham (ReeCocho)
 */

/** Includes. */
#include <Renderer.hpp>
#include <Physics.hpp>
#include <Threading.hpp>
#include <Scene.hpp>
#include <Clock.hpp>

#include "Input.hpp"
#include "ResourceManager.hpp"

namespace gust
{
	class Collider;

	struct CollisionData;

	/**
	 * @brief Startup the engine.
	 * @param Window name.
	 * @param Window width.
	 * @param Window height.
	 * @note This is called internally. Do not use.
	 */	
	extern void startup(const std::string& name, uint32_t width, uint32_t height);

	/**
	 * @brief Simulate the engine until completion.
	 * @note This is called internally. Do not use.
	 */
	extern void simulate();

	/**
	 * @brief Shutdown the engine.
	 * @note This is called internally. Do not use.
	 */
	extern void shutdown();

	/**
	 * @brief Get the frame rate.
	 * @return Frame rate.
	 */
	extern uint32_t getFrameRate();

	/**
	 * @brief Request all collisions involving the given collision object.
	 * @param Collision object.
	 * @return Collisions.
	 */
	extern const std::vector<PhysicsCollisionData>& requestCollisionData(btCollisionObject* obj);


	/** Graphics context. */
	extern Graphics graphics;

	/** Input manager. */
	extern Input input;

	/** Resource manager. */
	extern ResourceManager resourceManager;

	/** Rendering engine. */
	extern Renderer renderer;

	/** Physics engine. */
	extern Physics physics;

	/** Primary scene. */
	extern Scene scene;
}