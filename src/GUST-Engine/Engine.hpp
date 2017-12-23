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
	 * @brief Register a collision callback.
	 * @param Collider to check collisions for.
	 * @param Function to call.
	 */
	extern void registerCollisionCallback(Handle<Collider> collider, std::function<void(CollisionData)> callback);


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