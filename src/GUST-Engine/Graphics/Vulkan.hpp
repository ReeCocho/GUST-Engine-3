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
#include <SDL_syswm.h>
#include <SDL_vulkan.h>
#include <vulkan\vulkan.hpp>
#include "../Utilities/Math.hpp"

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

	/**
	 * @struct SwapChainBuffer
	 * @brief Individual swapchain buffer
	 */
	struct SwapChainBuffer
	{
		/** Swapchain buffers color image. */
		vk::Image image = {};

		/** Swapchain buffers views. */
		std::array<vk::ImageView, 2> views = {};

		/** Swapchain frame buffer. */
		vk::Framebuffer frameBuffer = {};
	};

	/**
	 * @struct VertexShaderData
	 * @brief Data that GUST sends to the vertex shader.
	 */
	struct VertexShaderData
	{
		/** Model-View-Projection Matrix. */
		glm::mat4 MVP = glm::mat4();

		/** Just the model matrix. */
		glm::mat4 model = glm::mat4();
	};


	/**
	 * @struct FragmentShaderData
	 * @brief Data that GUST sends to the fragment shader.
	 */
	struct FragmentShaderData
	{
		/** Where the camera is in world space coordinates. */
		glm::vec4 viewPosition = glm::vec4(0, 0, 0, 1);
	};

	/**
	 * @struct EmptyVertexData
	 * @brief Struct to use when not sending vertex data.
	 */
	struct EmptyVertexData
	{

	};

	/**
	 * @struct EmptyFragmentData
	 * @brief Struct to use when not sending fragment data.
	 */
	struct EmptyFragmentData
	{

	};

	/**
	 * @typedef SwapChainImages
	 * @brief Swap chain images type
	 */
	typedef std::vector<vk::Image, std::allocator<vk::Image>> SwapChainImages;
}