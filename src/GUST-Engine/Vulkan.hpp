#pragma once

/**
 * @file Vulkan.hpp
 * @brief Vulkan header file.
 * @author Connor J. Bramham (ReeCocho)
 */

// Tell SDL that the main function is handled.
#define SDL_MAIN_HANDLED

/** Includes. */
#include <SDL.h>
#include <SDL_vulkan.h>
#include <vulkan\vulkan.hpp>

namespace gust
{
	/**
	 * @struct Buffer
	 * @brief A Vulkan data buffer.
	 */
	struct Buffer
	{
		/** Memory. */
		vk::DeviceMemory memory = {};

		/** Buffer. */
		vk::Buffer buffer = {};
	};
}