#include "VulkanQueueManager.hpp"

namespace gust
{
	VulkanQueueManager::VulkanQueueManager(vk::Device logicalDevice, QueueFamilyIndices qfi)
	{
		m_graphicsQueue = logicalDevice.getQueue(static_cast<uint32_t>(qfi.graphicsFamily), 0);
		m_presentQueue = logicalDevice.getQueue(static_cast<uint32_t>(qfi.presentFamily), 0);
		m_transferQueue = logicalDevice.getQueue(static_cast<uint32_t>(qfi.transferFamily), 0);

		assert(m_graphicsQueue);
		assert(m_presentQueue);
		assert(m_transferQueue);
	}

	VulkanQueueManager::~VulkanQueueManager()
	{

	}
}