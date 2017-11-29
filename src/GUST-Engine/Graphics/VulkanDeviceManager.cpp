#include <map>
#include <set>
#include "../Core/Debugging.hpp"
#include "VulkanDeviceManager.hpp"

namespace gust
{
	VulkanDeviceManager::VulkanDeviceManager
	(
		vk::Instance instance, 
		VulkanSurfaceManager* surfaceManager, 
		const std::vector<const char*>& layers, 
		const std::vector<const char*>& extensions
	) :
		m_instance(instance),
		m_surfaceManager(surfaceManager),
		m_layers(layers),
		m_extensions(extensions)
	{
		initPhysicalDevice();
		initLogicalDevice();
	}

	VulkanDeviceManager::~VulkanDeviceManager()
	{
		// Destroy logical device
		m_logicalDevice.destroy();
	}

	void VulkanDeviceManager::initPhysicalDevice()
	{
		// Get physical devices
		auto devices = m_instance.enumeratePhysicalDevices();
		assert(devices.size() > 0);

		// Map that holds GPU's and their scores
		std::multimap<size_t, std::tuple<vk::PhysicalDevice, QueueFamilyIndices>> canidates = {};

		// Find suitable device
		for (const auto& device : devices)
		{
			const auto score = getDeviceScore(device);
			const auto qfi = findQueueFamilies(device);
			canidates.insert(std::make_pair(score, std::make_tuple(device, qfi)));
		}

		// Pick best device
		if (canidates.rbegin()->first == 0 || !std::get<1>(canidates.rbegin()->second).isComplete())
			throwError("VULKAN: Unable to find physical device canidate.");

		m_physicalDevice = std::get<0>(canidates.rbegin()->second);
		m_queueFamilyIndices = std::get<1>(canidates.rbegin()->second);
		assert(m_physicalDevice);
	}

	void VulkanDeviceManager::initLogicalDevice()
	{
		// Get GPU indices
		std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos = {};
		const std::set<int> uniqueQueueFamilies =
		{
			m_queueFamilyIndices.graphicsFamily,
			m_queueFamilyIndices.presentFamily,
			m_queueFamilyIndices.transferFamily
		};

		// Get queue creation info
		const float queuePriority = 1.0f;
		for (const auto queueFamily : uniqueQueueFamilies)
		{
			vk::DeviceQueueCreateInfo queueCreateInfo = {};
			queueCreateInfo.setQueueFamilyIndex(queueFamily);
			queueCreateInfo.setQueueCount(1);
			queueCreateInfo.setPQueuePriorities(&queuePriority);
			queueCreateInfos.push_back(queueCreateInfo);
		}

		// Requested features for the device
		vk::PhysicalDeviceFeatures deviceFeatures = {};

		// Logical device creation info
		vk::DeviceCreateInfo createInfo = {};
		createInfo.setFlags(vk::DeviceCreateFlags());
		createInfo.setPQueueCreateInfos(queueCreateInfos.data());								// Queue creation info
		createInfo.setQueueCreateInfoCount(static_cast<uint32_t>(queueCreateInfos.size()));		// Number of queues
		createInfo.setPEnabledFeatures(&deviceFeatures);										// Requested device features
		createInfo.setPpEnabledLayerNames(m_layers.data());										// Validation layers
		createInfo.setEnabledLayerCount(static_cast<uint32_t>(m_layers.size()));				// Validation layer count
		createInfo.setPpEnabledExtensionNames(m_deviceExtensions.data());						// Extensions
		createInfo.setEnabledExtensionCount(static_cast<uint32_t>(m_deviceExtensions.size()));	// Extension count

		// Create logical device
		if (m_physicalDevice.createDevice(&createInfo, nullptr, &m_logicalDevice) != vk::Result::eSuccess)
			throwError("VULKAN: Unable to create logical device.");
	}

	size_t VulkanDeviceManager::getDeviceScore(vk::PhysicalDevice device)
	{
		// Get physical device properties
		const auto deviceProps = device.getProperties();

		// Get physical device features
		const auto deviceFeatures = device.getFeatures();

		// Device score
		size_t score = 0;

		// Prefer discrete GPU's
		if (deviceProps.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
			score += 2500;

		// Prefer higher image dimensions
		score += deviceProps.limits.maxImageDimension2D;

		// Requires geometry shaders
		if (!deviceFeatures.geometryShader)
			score = 0;

		// Supports device extensions
		if (!supportsDeviceExtensions(device))
			score = 0;

		return score;
	}

	QueueFamilyIndices VulkanDeviceManager::findQueueFamilies(vk::PhysicalDevice device)
	{
		QueueFamilyIndices indices = {};

		// Get queue families
		const auto queueFamilies = device.getQueueFamilyProperties();

		// Find optimal queue families
		int i = 0;
		for (const auto& queueFamily : queueFamilies)
		{
			vk::Bool32 presentSupport = device.getSurfaceSupportKHR(i, m_surfaceManager->getSurface());

			// Check for graphics family
			if (queueFamily.queueCount > 0 && queueFamily.queueFlags & vk::QueueFlagBits::eGraphics)
				indices.graphicsFamily = i;

			// Check for present family
			if (queueFamily.queueCount > 0 && presentSupport)
				indices.presentFamily = i;

			// Check for transfer family
			if (queueFamily.queueCount > 0 && !(queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) && queueFamily.queueFlags & vk::QueueFlagBits::eTransfer)
				indices.transferFamily = i;

			// Check if we have all indices
			if (indices.isComplete())
				break;

			// Increment index counter
			i++;
		}

		return indices;
	}

	bool VulkanDeviceManager::supportsDeviceExtensions(vk::PhysicalDevice physicalDevice)
	{
		// Extensions avaliable
		const auto availableExtensions = physicalDevice.enumerateDeviceExtensionProperties();

		std::set<std::string> requiredExtensions(m_deviceExtensions.begin(), m_deviceExtensions.end());

		for (const auto& extension : availableExtensions)
			requiredExtensions.erase(extension.extensionName);

		return requiredExtensions.empty();
	}
}