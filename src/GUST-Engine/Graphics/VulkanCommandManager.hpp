#pragma once

/** 
 * @file VulkanCommandManager.hpp
 * @brief Vulkan command manager header file.
 * @author Connor J. Bramham (ReeCocho)
 */

/** Includes. */
#include "VulkanDeviceManager.hpp"
#include "VulkanQueueManager.hpp"

namespace gust
{
	/**
	 * @class VulkanCommandManager
	 * @brief Manages command buffers and command pools.
	 */
	class VulkanCommandManager
	{
	public:

		/**
		 * @brief Default constructor.
		 */
		VulkanCommandManager() = default;

		/**
		 * @brief Constructor.
		 * @param Device manager.
		 * @param Queue manager.
		 */
		VulkanCommandManager(VulkanDeviceManager* deviceManager, VulkanQueueManager* queueManager);

		/**
		 * @brief Destructor.
		 */
		~VulkanCommandManager();

		/**
		 * @brief Create a command buffer.
		 * @param Command buffer level.
		 * @return New command buffer.
		 */
		vk::CommandBuffer createCommandBuffer(vk::CommandBufferLevel level);

		/**
		 * @brief Destroy a command buffer.
		 * @param Command buffer to destroy.
		 */
		inline void destroyCommandBuffer(vk::CommandBuffer commandBuffer)
		{
			m_deviceManager->getLogicalDevice().freeCommandBuffers(m_graphicsPool, commandBuffer);
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
		 * @brief Reset graphics command buffers.
		 * @note Used internally. Do not call.
		 */
		void resetGraphicsCommandBuffers();

	private:
		
		/** Device manager. */
		VulkanDeviceManager* m_deviceManager = nullptr;

		/** Queue manager. */
		VulkanQueueManager* m_queueManager = nullptr;

		/** Graphics command pool. */
		vk::CommandPool m_graphicsPool = {};

		/** Transfer command pool. */
		vk::CommandPool m_transferPool = {};

		/** Single use command pool. */
		vk::CommandPool m_singleUsePool = {};
	};
}