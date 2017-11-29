#pragma once

/**
 * @file VulkanDeviceManager.hpp
 * @brief Vulkan device manager header file.
 * @author Connor J. Bramham (ReeCocho)
 */

/** Includes. */
#include <string>
#include <vector>
#include "Vulkan.hpp"
#include "VulkanSurfaceManager.hpp"



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
	 * @class VulkanDeviceManager
	 * @brief Manages Vulkan physical and logical devices.
	 * @see Graphics
	 */
	class VulkanDeviceManager
	{
	public:

		/**
		 * @brief Default constructor.
		 */
		VulkanDeviceManager() = default;

		/**
		 * @brief Constructor.
		 * @param Vulkan instance.
		 * @param Surface manager.
		 * @param Layers.
		 * @param Extensions.
		 */
		VulkanDeviceManager
		(
			vk::Instance instance, 
			VulkanSurfaceManager* surfaceManager, 
			const std::vector<const char*>& layers, 
			const std::vector<const char*>& extensions
		);

		/**
		 * @brief Destructor.
		 */
		~VulkanDeviceManager();



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
		inline const QueueFamilyIndices getQueueFamilyIndices() const
		{
			return m_queueFamilyIndices;
		}

	private:

		/**
		 * @brief Initialize Vulkan physical device.
		 */
		void initPhysicalDevice();

		/**
		 * @brief Initialize Vulkan logical device.
		 */
		void initLogicalDevice();

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



		/** Vulkan layers */
		const std::vector<const char*> m_layers;

		/** Vulkan extensions. */
		const std::vector<const char*> m_extensions;

		/** Vulkan device extensions. */
		std::vector<const char*> m_deviceExtensions =
		{
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

		/** Vulkan instance. */
		const vk::Instance m_instance;

		/** Vulkan physical device. */
		vk::PhysicalDevice m_physicalDevice = {};

		/** Surface manager. */
		VulkanSurfaceManager* m_surfaceManager = nullptr;

		/** Vulkan logical device. */
		vk::Device m_logicalDevice = {};

		/** Physical device queue family indices. */
		QueueFamilyIndices m_queueFamilyIndices = {};
	};
}