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
#include "Math.hpp"
#include "VulkanDebugging.hpp"

namespace gust
{
	/**
	 * @struct QueueFamilyIndices
	 * @brief Stores queue family indices for different queue types.
	 */
	struct QueueFamilyIndices
	{
		/** Index of graphics family. */
		int graphicsFamily = -1;

		/** Index of present family. */
		int presentFamily = -1;

		/** Index of transfer family. */
		int transferFamily = -1;

		bool isComplete()
		{
			return graphicsFamily >= 0 && presentFamily >= 0 && transferFamily >= 0;
		}
	};



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
		 * @brief Get physical device.
		 * @return Physical device.
		 */
		inline vk::PhysicalDevice getPhysicalDevice() const
		{
			return m_physicalDevice;
		}

		/**
		 * @brief Get logical device.
		 * @return Logical device.
		 */
		inline vk::Device getLogicalDevice() const
		{
			return m_logicalDevice;
		}

		/**
		 * @brief Get queue family indices.
		 * @return Queue family indices.
		 */
		inline QueueFamilyIndices getQueueFamilyIndices() const
		{
			return m_queueFamilyIndices;
		}

		/**
		 * @brief Get transfer command pool.
		 * @return Transfer command pool.
		 */
		inline const vk::CommandPool& getTransferPool() const
		{
			return m_transferPool;
		}

		/**
		 * @brief Create a command buffer for a single use.
		 * @return Single use command buffer.
		 */
		vk::CommandBuffer beginSingleTimeCommands();

		/**
		 * @brief End single use command buffer.
		 * @param Single use command buffer to end.
		 */
		void endSingleTimeCommands(vk::CommandBuffer commandBuffer);

		/**
		 * @brief Get graphics queue.
		 * @return Graphics queue.
		 */
		inline vk::Queue getGraphicsQueue() const
		{
			return m_graphicsQueue;
		}

		/**
		 * @brief Get presentation queue.
		 * @return Presentation queue.
		 */
		inline vk::Queue getPresentationQueue() const
		{
			return m_presentQueue;
		}

		/**
		 * @brief Get transfer queue.
		 * @return Transfer queue.
		 */
		inline vk::Queue getTransferQueue() const
		{
			return m_transferQueue;
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
		 * @param Number of images.
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
			vk::DeviceMemory& imageMemory,
			vk::ImageCreateFlags flags = static_cast<vk::ImageCreateFlagBits>(0),
			uint32_t arrayLayers = 1
		);

		/**
		 * @brief Create transition image layout.
		 * @param Image to create transition layout for.
		 * @param Transition format.
		 * @param Old image layout.
		 * @param New image layout.
		 * @param Image count.
		 */
		void transitionImageLayout
		(
			const vk::Image& image, 
			vk::Format format, 
			vk::ImageLayout oldLayout, 
			vk::ImageLayout newLayout,
			uint32_t imageCount = 1
		);

		/**
		 * @brief Copy a buffer to an image.
		 * @param Buffer to copy from.
		 * @param Image to copy to.
		 * @param Image width.
		 * @param Image height.
		 * @param Number of images to copy.
		 * @param Size of each image.
		 * @note The image size can be zero if you only have one image to copy.
		 */
		void copyBufferToImage
		(
			const vk::Buffer& buffer, 
			const vk::Image& image, 
			uint32_t width, 
			uint32_t height,
			uint32_t imageCount = 1,
			uint32_t imageSize = 0
		);

		/**
		 * @brief Create image view.
		 * @param Image to create view for.
		 * @param Image view format.
		 * @param Image aspect flags.
		 * @param Image view type.
		 * @param Number of images.
		 * @return New image view.
		 */
		vk::ImageView createImageView
		(
			const vk::Image& image, 
			vk::Format format, 
			vk::ImageAspectFlags aspectFlags,
			vk::ImageViewType viewType = vk::ImageViewType::e2D,
			uint32_t imageCount = 1
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

		/**
		 * @brief Gets the rating of a physical device.
		 * @param Device to check.
		 * @return Device score.
		 */
		size_t getDeviceScore(vk::PhysicalDevice device);

		/**
		 * @brief Get queue family indices of a physical device.
		 * @param Physical device to check.
		 * @return Queue family indices of the device.
		 */
		QueueFamilyIndices findQueueFamilies(vk::PhysicalDevice device);

		/**
		 * @brief Check if device supports device extensions
		 * @param Physical device to check extensions of.
		 * @return If the device supports device extensions.
		 */
		bool supportsDeviceExtensions(vk::PhysicalDevice physicalDevice);

		/**
		 * @brief Initialize Vulkan surface formats.
		 * @param Physical device needed too check if it supports certain formats.
		 */
		void initSurfaceFormats(vk::PhysicalDevice physicalDevice);



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

		/** Vulkan device extensions. */
		std::vector<const char*> m_deviceExtensions =
		{
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

#ifndef NDEBUG
		/** Vulkan debugging manager. */
		std::unique_ptr<VulkanDebugging> m_debugging;
#endif

		/** Memory allocator. */
		VmaAllocator m_memoryAllocator = {};

		/** Vulkan surface. */
		vk::SurfaceKHR m_surface = {};

		/** Surface color format. */
		vk::Format m_colorFormat = {};

		/** Surface color space. */
		vk::ColorSpaceKHR m_colorSpace = {};

		/** Surface depth format. */
		vk::Format m_depthFormat = {};

		/** Vulkan physical device. */
		vk::PhysicalDevice m_physicalDevice = {};

		/** Vulkan logical device. */
		vk::Device m_logicalDevice = {};

		/** Physical device queue family indices. */
		QueueFamilyIndices m_queueFamilyIndices = {};

		/** Graphics queue. */
		vk::Queue m_graphicsQueue = {};

		/** Presentation queue. */
		vk::Queue m_presentQueue = {};

		/** Transfer queue. */
		vk::Queue m_transferQueue = {};

		/** Transfer command pool. */
		vk::CommandPool m_transferPool = {};

		/** Single use command pool. */
		vk::CommandPool m_singleUsePool = {};
	};
}