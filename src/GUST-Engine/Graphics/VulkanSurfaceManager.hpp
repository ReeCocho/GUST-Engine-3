#pragma once

/**
 * @file VulkanSurfaceManager.hpp
 * @brief Vulkan surface manager header file.
 * @author Connor J. Bramham (ReeCocho)
 */

/** Includes. */
#include "Vulkan.hpp"

namespace gust
{
	/**
	 * @class VulkanSurfaceManager
	 * @brief Manages a Vulkan surface and SDL window.
	 */
	class VulkanSurfaceManager
	{
	public:

		/**
		 * @brief Default constructor.
		 */
		VulkanSurfaceManager() = default;

		/**
		 * @brief Constructor.
		 * @param Vulkan instance.
		 * @param Window.
		 */
		VulkanSurfaceManager
		(
			vk::Instance instance,
			SDL_Window* window
		);

		/**
		 * @brief Destructor.
		 */
		~VulkanSurfaceManager();

		/**
		 * @brief Get surface.
		 * @return Surface.
		 */
		inline const vk::SurfaceKHR& getSurface() const
		{
			return m_surface;
		}

		/**
		 * @brief Get surface color format.
		 * @return Surface color format.
		 */
		inline vk::Format getSurfaceColorFormat() const
		{
			return m_colorFormat;
		}

		/**
		 * @brief Get surface color space.
		 * @return Surface color space.
		 */
		inline vk::ColorSpaceKHR getSurfaceColorSpace() const
		{
			return m_colorSpace;
		}

		/**
		 * @brief Get depth format.
		 * @return Depth format.
		 */
		inline vk::Format getDepthFormat() const
		{
			return m_depthFormat;
		}

		/**
		 * @brief Initialize Vulkan surface formats.
		 * @param Physical device needed too check if it supports certain formats.
		 */
		void initSurfaceFormats(vk::PhysicalDevice physicalDevice);

	private:

		/** Vulkan surface. */
		vk::SurfaceKHR m_surface = {};

		/** Surface color format. */
		vk::Format m_colorFormat = {};

		/** Surface color space. */
		vk::ColorSpaceKHR m_colorSpace = {};

		/** Surface depth format. */
		vk::Format m_depthFormat = {};

		/** Vulkan instance. */
		const vk::Instance m_instance;
	};
}