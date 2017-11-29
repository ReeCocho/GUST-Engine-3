#include "VulkanCommandManager.hpp"

namespace gust
{
	VulkanCommandManager::VulkanCommandManager(VulkanDeviceManager* deviceManager, VulkanQueueManager* queueManager) : 
		m_deviceManager(deviceManager),
		m_queueManager(queueManager)
	{
		auto logicalDevice = m_deviceManager->getLogicalDevice();

		// Create a command pools
		auto commandPoolInfo = vk::CommandPoolCreateInfo
		(
			vk::CommandPoolCreateFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer),
			m_deviceManager->getQueueFamilyIndices().graphicsFamily
		);

		m_graphicsPool = logicalDevice.createCommandPool(commandPoolInfo);

		commandPoolInfo = vk::CommandPoolCreateInfo
		(
			vk::CommandPoolCreateFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer),
			m_deviceManager->getQueueFamilyIndices().transferFamily
		);

		// Create transfer pool
		m_transferPool = logicalDevice.createCommandPool(commandPoolInfo);

		commandPoolInfo = vk::CommandPoolCreateInfo
		(
			vk::CommandPoolCreateFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer),
			m_deviceManager->getQueueFamilyIndices().graphicsFamily
		);

		// Create single use pool
		m_singleUsePool = logicalDevice.createCommandPool(commandPoolInfo);
	}

	VulkanCommandManager::~VulkanCommandManager()
	{
		auto logicalDevice = m_deviceManager->getLogicalDevice();

		// Cleanup pools
		logicalDevice.destroyCommandPool(m_graphicsPool);
		logicalDevice.destroyCommandPool(m_transferPool);
		logicalDevice.destroyCommandPool(m_singleUsePool);
	}

	vk::CommandBuffer VulkanCommandManager::createCommandBuffer(vk::CommandBufferLevel level)
	{
		// Allocate command buffer
		auto commandBuffer = m_deviceManager->getLogicalDevice().allocateCommandBuffers
		(
			vk::CommandBufferAllocateInfo
			(
				m_graphicsPool,
				level,
				1
			)
		);

		return commandBuffer[0];
	}

	void VulkanCommandManager::resetGraphicsCommandBuffers()
	{
		m_deviceManager->getLogicalDevice().resetCommandPool(m_graphicsPool, vk::CommandPoolResetFlagBits::eReleaseResources);
	}

	vk::CommandBuffer VulkanCommandManager::beginSingleTimeCommands()
	{
		vk::CommandBufferAllocateInfo allocInfo = {};
		allocInfo.setLevel(vk::CommandBufferLevel::ePrimary);
		allocInfo.setCommandPool(m_singleUsePool);
		allocInfo.setCommandBufferCount(1);

		auto commandBuffer = m_deviceManager->getLogicalDevice().allocateCommandBuffers(allocInfo)[0];

		vk::CommandBufferBeginInfo beginInfo = {};
		beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

		commandBuffer.begin(beginInfo);

		return commandBuffer;
	}

	void VulkanCommandManager::endSingleTimeCommands(vk::CommandBuffer commandBuffer)
	{
		commandBuffer.end();

		vk::SubmitInfo submitInfo = {};
		submitInfo.setCommandBufferCount(1);
		submitInfo.setPCommandBuffers(&commandBuffer);

		m_queueManager->getGraphicsQueue().submit(1, &submitInfo, { nullptr });
		m_queueManager->getGraphicsQueue().waitIdle();

		m_deviceManager->getLogicalDevice().freeCommandBuffers(m_singleUsePool, 1, &commandBuffer);
	}
}