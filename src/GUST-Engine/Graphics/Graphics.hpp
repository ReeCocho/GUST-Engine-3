#pragma once

/** 
 * @file Graphics.hpp
 * @brief Graphics header file.
 * @author Connor J. Bramham (ReeCocho)
 */

 /** Includes. */
#include <string>
#include <vector>

#include "Vulkan.hpp"
#include "../Utilities/Math.hpp"
#include "VulkanDebugging.hpp"
#include "VulkanSurfaceManager.hpp"
#include "VulkanDeviceManager.hpp"
#include "VulkanQueueManager.hpp"
#include "VulkanCommandManager.hpp"

namespace gust
{
	/**
	 * @class Graphics
	 * @brief Manages interacting with Vulkan and a window.
	 */
	class Graphics
	{
	public:

		/**
		 * @brief Default constructor.
		 */
		Graphics() = default;

		/**
		 * @brief Default destructor.
		 */
		~Graphics() = default;

		/**
		 * @brief Startup graphics.
		 * @param Window name.
		 * @param Window width.
		 * @param Window height.
		 * @note This is called internally. Do not use.
		 */
		void startup(const std::string& name, uint32_t width, uint32_t height);

		/**
		 * @brief Shutdown graphics.
		 * @note This is called internally. Do not use.
		 */
		void shutdown();

		/**
		 * @brief Set resolution.
		 * @param New width.
		 * @param New height.
		 * @note If the graphics context is being used by a renderer you must change resolution on the renderer.
		 */
		void setResolution(uint32_t width, uint32_t height);

		/** 
		 * @brief Get window width.
		 * @return Window width.
		 */
		inline uint32_t getWidth() const
		{
			return m_width;
		}

		/**
		 * @brief Get window height.
		 * @return Window height.
		 */
		inline uint32_t getHeight() const
		{
			return m_height;
		}

		/**
		 * @brief Get surface manager.
		 * @return Surface manager.
		 */
		inline VulkanSurfaceManager* getSurface() const
		{
			return m_surfaceManager.get();
		}

		/**
		 * @brief Get device manager.
		 * @return Device manager.
		 */
		inline VulkanDeviceManager* getDeviceManager() const
		{
			return m_deviceManager.get();
		}

		/**
		 * @brief Get command manager.
		 * @return Command manager.
		 */
		inline VulkanCommandManager* getCommandManager() const
		{
			return m_commandManager.get();
		}

		/**
		 * @brief Get queue manager.
		 * @return Queue manager.
		 */
		inline VulkanQueueManager* getQueueManager() const
		{
			return m_queueManager.get();
		}

		/**
		 * @brief Create buffer.
		 * @param Size of data to buffer.
		 * @param Usage flags.
		 * @param Memory properties flags.
		 * @return Buffer.
		 */
		Buffer createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties);

		/**
		 * @brief Copy buffer.
		 * @param Source buffer.
		 * @param Destination buffer.
		 * @param Size of source buffer.
		 */
		void copyBuffer(const vk::Buffer& sourceBuffer, const vk::Buffer& destinationBuffer, vk::DeviceSize size);

		/**
		 * @ brief Find memory type based off memory properties.
		 * @param Type filter.
		 * @param Memory properties.
		 * @return Memory type.
		 */
		uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);

		/**
		 * @brief Create image.
		 * @param Image width.
		 * @param Image height.
		 * @param Format.
		 * @param Tiling.
		 * @param Usage.
		 * @param Memory property flags.
		 * @param Image reference.
		 * @param Image memory reference.
		 */
		void createImage
		(
			uint32_t width,
			uint32_t height,
			vk::Format format,
			vk::ImageTiling tiling,
			vk::ImageUsageFlags usage,
			vk::MemoryPropertyFlags properties,
			vk::Image& image,
			vk::DeviceMemory& imageMemory
		);

		/**
		 * @brief Create transition image layout.
		 * @param Image to create transition layout for.
		 * @param Transition format.
		 * @param Old image layout.
		 * @param New image layout.
		 */
		void transitionImageLayout
		(
			const vk::Image& image, 
			vk::Format format, 
			vk::ImageLayout oldLayout, 
			vk::ImageLayout newLayout
		);

		/**
		 * @brief Copy a buffer to an image.
		 * @param Buffer to copy from.
		 * @param Image to copy to.
		 * @param Image width.
		 * @param Image height.
		 */
		void copyBufferToImage
		(
			const vk::Buffer& buffer, 
			const vk::Image& image, 
			uint32_t width, 
			uint32_t height
		);

		/**
		 * @brief Create image view.
		 * @param Image to create view for.
		 * @param Image view format.
		 * @param Image aspect flags.
		 * @return New image view.
		 */
		vk::ImageView createImageView
		(
			const vk::Image& image, 
			vk::Format format, 
			vk::ImageAspectFlags aspectFlags
		);

	private:

		/**
		 * @brief Get layers supported by Vulkan.
		 * @param Layers requested.
		 * @param Layers found.
		 */
		std::vector<const char*> getLayers(const std::vector<const char*>& layers);

		/**
		 * @brief Get extensions supported by Vulkan.
		 * @param Extensions requested.
		 * @param Extensions found.
		 */
		std::vector<const char*> getExtensions(const std::vector<const char*>& extensions);



		/** SDL Window. */
		SDL_Window* m_window;

		/** Window width. */
		uint32_t m_width;

		/** Window height. */
		uint32_t m_height;

		/** Window name. */
		std::string m_name;

		/** Vulkan instance. */
		vk::Instance m_instance;

		/** Vulkan layers */
		std::vector<const char*> m_layers;

		/** Vulkan extensions. */
		std::vector<const char*> m_extensions;

#ifndef NDEBUG
		/** Vulkan debugging manager. */
		std::unique_ptr<VulkanDebugging> m_debugging;
#endif

		/** Vulkan surface manager. */
		std::unique_ptr<VulkanSurfaceManager> m_surfaceManager;

		/** Vulkan device manager. */
		std::unique_ptr<VulkanDeviceManager> m_deviceManager;

		/** Vulkan queue manager. */
		std::unique_ptr<VulkanQueueManager> m_queueManager;

		/** Vulkan command manager. */
		std::unique_ptr<VulkanCommandManager> m_commandManager;
	};
}