#pragma once

/**
 * @file VulkanQueueManager.hpp
 * @brief Vulkan queue manager header file.
 * @author Connor J. Bramham (ReeCocho)
 */

/** Includes. */
#include "VulkanDeviceManager.hpp"

namespace gust
{
	/**
	 * @class VulkanQueueManager
	 * @brief Manages queues created for a logical device.
	 */
	class VulkanQueueManager
	{
	public:

		/**
		 * @brief Default constructor.
		 */
		VulkanQueueManager() = default;

		/**
		 * @brief Constructor.
		 * @param Logical device.
		 * @param Queue family indices.
		 */
		VulkanQueueManager(vk::Device logicalDevice, QueueFamilyIndices qfi);

		/**
		 * @brief Destructor.
		 */
		~VulkanQueueManager();



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

	private:

		/** Graphics queue. */
		vk::Queue m_graphicsQueue = {};

		/** Presentation queue. */
		vk::Queue m_presentQueue = {};

		/** Transfer queue. */
		vk::Queue m_transferQueue = {};
	};
}