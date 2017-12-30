#include <array>
#include <map>
#include <set>
#include <Debugging.hpp>
#include "Graphics.hpp"

namespace gust
{
	void Graphics::startup(const std::string& name, uint32_t width, uint32_t height)
	{
		// Check parameters
		assert(width > 0);
		assert(height > 0);
		assert(name != "");

		m_width = width;
		m_height = height;
		m_name = name;

		// Initialize SDL video
		if (SDL_Init(SDL_INIT_VIDEO | SDL_VIDEO_VULKAN) != 0)
			throwError("SDL: Unable to initialize video.");

		// Create SDL window
		m_window = SDL_CreateWindow
		(
			m_name.c_str(),
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED,
			m_width,
			m_height,
			SDL_WINDOW_VULKAN | SDL_WINDOW_SHOWN
		);

		// Check window creation
		if (m_window == nullptr)
			throwError("SDL: Unable to create window.");

		// Extension data
		const char** extensions = nullptr;
		unsigned extensionCount = 0;

		if (!SDL_Vulkan_GetInstanceExtensions(m_window, &extensionCount, nullptr))
			throwError("SDL: Unable to get extension count.");

		extensions = static_cast<const char**>(SDL_malloc(sizeof(const char*) * extensionCount));

		if (!SDL_Vulkan_GetInstanceExtensions(m_window, &extensionCount, extensions))
			throwError("SDL: Unable to get extension names.");

		// Requested layers
		std::vector<const char*> requestedLayers =
		{
#ifndef NDEBUG
			"VK_LAYER_LUNARG_standard_validation"		
#endif
		};

		// Requested extensions
		std::vector<const char*> requestedExtensions =
		{
			// Debugging
#ifndef NDEBUG
			VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
#endif
			VK_KHR_SURFACE_EXTENSION_NAME
		};

		// Add extensions required by SDL
		for (size_t i = 0; i < extensionCount; ++i)
			requestedExtensions.push_back(extensions[i]);

		// Describes application information
		vk::ApplicationInfo appInfo = {};
		appInfo.setPApplicationName(name.c_str());					// Application name
		appInfo.setApplicationVersion(VK_MAKE_VERSION(1, 0, 0));	// Application version
		appInfo.setPEngineName("GUST Engine");						// Engine name
		appInfo.setEngineVersion(VK_MAKE_VERSION(1, 0, 0));			// Engine version
		appInfo.setApiVersion(VK_API_VERSION_1_0);					// Vulkan version

		// Get extensions and layers
		m_layers = getLayers(requestedLayers);
		assert(m_layers.size() == requestedLayers.size());
		
		m_extensions = getExtensions(requestedExtensions);
		assert(m_extensions.size() == requestedExtensions.size());

		// Instance creation info
		vk::InstanceCreateInfo createInfo = {};
		createInfo.setFlags(vk::InstanceCreateFlags());											// Creation flags
		createInfo.setPApplicationInfo(&appInfo);												// Application info pointer
		createInfo.setEnabledLayerCount(static_cast<uint32_t>(m_layers.size()));				// Number of layers
		createInfo.setPpEnabledLayerNames(m_layers.data());										// Layer data
		createInfo.setEnabledExtensionCount(static_cast<uint32_t>(m_extensions.size()));		// Number of extensions
		createInfo.setPpEnabledExtensionNames(m_extensions.data());								// Extension data

		// Create Vulkan instance
		if (vk::createInstance(&createInfo, nullptr, &m_instance) != vk::Result::eSuccess)
			throwError("VULKAN: Unable to create instance.");

		// Free extension memory
		SDL_free(static_cast<void*>(extensions));

#ifndef NDEBUG
		// Create debugging manager
		m_debugging = std::make_unique<VulkanDebugging>(m_instance);
#endif

		// Create surface
		{
			// Create Vulkan surface.
			VkSurfaceKHR surface = VK_NULL_HANDLE;
			SDL_Vulkan_CreateSurface(m_window, static_cast<VkInstance>(m_instance), &surface);

			assert(surface != VK_NULL_HANDLE);

			m_surface = static_cast<vk::SurfaceKHR>(surface);
		}

		// Create devices
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

		// Initialize surface formats
		initSurfaceFormats(m_physicalDevice);

		// Check if the physical device supports the surface.
		if (!m_physicalDevice.getSurfaceSupportKHR(m_queueFamilyIndices.presentFamily, m_surface))
			throwError("VULKAN: Physical device does not support presenting to the surface.");

		// Create queues
		{
			m_graphicsQueue = m_logicalDevice.getQueue(static_cast<uint32_t>(m_queueFamilyIndices.graphicsFamily), 0);
			m_presentQueue = m_logicalDevice.getQueue(static_cast<uint32_t>(m_queueFamilyIndices.presentFamily), 0);
			m_transferQueue = m_logicalDevice.getQueue(static_cast<uint32_t>(m_queueFamilyIndices.transferFamily), 0);

			assert(m_graphicsQueue);
			assert(m_presentQueue);
			assert(m_transferQueue);
		}

		// Create command stuff
		{
			// Create a command pools
			auto commandPoolInfo = vk::CommandPoolCreateInfo
			(
				vk::CommandPoolCreateFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer),
				m_queueFamilyIndices.transferFamily
			);

			// Create transfer pool
			m_transferPool = m_logicalDevice.createCommandPool(commandPoolInfo);

			commandPoolInfo = vk::CommandPoolCreateInfo
			(
				vk::CommandPoolCreateFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer),
				m_queueFamilyIndices.graphicsFamily
			);

			// Create single use pool
			m_singleUsePool = m_logicalDevice.createCommandPool(commandPoolInfo);
		}
	}

	void Graphics::shutdown()
	{
		m_logicalDevice.waitIdle();

		// Cleanup pools
		m_logicalDevice.destroyCommandPool(m_transferPool);
		m_logicalDevice.destroyCommandPool(m_singleUsePool);

		// Destroy logical device
		m_logicalDevice.destroy();

		// Destroy surface
		m_instance.destroySurfaceKHR(m_surface);

		// Cleanup window
		SDL_DestroyWindow(m_window);

#ifndef NDEBUG
		// Destroy debugger
		m_debugging = nullptr;
#endif

		// Destroy instance
		m_instance.destroy();
	}

	void Graphics::setResolution(uint32_t width, uint32_t height)
	{
		m_logicalDevice.waitIdle();

		assert(m_width > 0);
		assert(m_height > 0);

		m_width = width;
		m_height = height;
		SDL_SetWindowSize(m_window, m_width, m_height);
	}

	Buffer Graphics::createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties)
	{
		// Buffer to return
		Buffer buffer = {};

		std::array<uint32_t, 2> queues =
		{
			static_cast<uint32_t>(m_queueFamilyIndices.graphicsFamily),
			static_cast<uint32_t>(m_queueFamilyIndices.transferFamily)
		};

		vk::BufferCreateInfo bufferInfo = {};
		bufferInfo.setSize(size);
		bufferInfo.setUsage(usage);
		bufferInfo.setQueueFamilyIndexCount(2);
		bufferInfo.setPQueueFamilyIndices(queues.data());
		bufferInfo.setSharingMode(vk::SharingMode::eConcurrent);

		// Create buffer
		buffer.buffer = m_logicalDevice.createBuffer(bufferInfo);

		vk::MemoryRequirements memRequirements = {};
		m_logicalDevice.getBufferMemoryRequirements(buffer.buffer, &memRequirements);

		vk::MemoryAllocateInfo allocInfo = {};
		allocInfo.setAllocationSize(memRequirements.size);
		allocInfo.setMemoryTypeIndex(findMemoryType(memRequirements.memoryTypeBits, properties));

		// Allocate memory
		buffer.memory = m_logicalDevice.allocateMemory(allocInfo);

		// Bind memory
		m_logicalDevice.bindBufferMemory(buffer.buffer, buffer.memory, 0);

		return buffer;
	}

	void Graphics::copyBuffer(const vk::Buffer& sourceBuffer, const vk::Buffer& destinationBuffer, vk::DeviceSize size)
	{
		vk::CommandBufferAllocateInfo allocInfo = {};
		allocInfo.setLevel(vk::CommandBufferLevel::ePrimary);
		allocInfo.setCommandPool(m_transferPool);
		allocInfo.setCommandBufferCount(1);

		// Create command buffer
		vk::CommandBuffer commandBuffer = {};
		commandBuffer = m_logicalDevice.allocateCommandBuffers(allocInfo)[0];

		vk::CommandBufferBeginInfo beginInfo = {};
		beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

		// Start recording to command buffer
		commandBuffer.begin(beginInfo);

		vk::BufferCopy copyRegion = {};
		copyRegion.setSrcOffset(0);
		copyRegion.setDstOffset(0);
		copyRegion.setSize(size);

		// Copy srcBuffer to dstBuffer
		commandBuffer.copyBuffer(sourceBuffer, destinationBuffer, 1, &copyRegion);

		// Stop recording to command buffer
		commandBuffer.end();

		vk::SubmitInfo submitInfo = {};
		submitInfo.setCommandBufferCount(1);
		submitInfo.setPCommandBuffers(&commandBuffer);

		// Subit command buffer
		m_transferQueue.submit(1, &submitInfo, {});
		m_transferQueue.waitIdle();

		// Destroy command buffer
		m_logicalDevice.freeCommandBuffers(m_transferPool, 1, &commandBuffer);
	}

	uint32_t Graphics::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties)
	{
		// Qeury memory properties
		auto memProperties = m_physicalDevice.getMemoryProperties();

		// Find memory type
		for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i)
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
				return i;

		throwError("VULKAN: Failed to find suitable memory type");
		return 0;
	}

	void Graphics::createImage
	(
		uint32_t width,
		uint32_t height,
		vk::Format format,
		vk::ImageTiling tiling,
		vk::ImageUsageFlags usage,
		vk::MemoryPropertyFlags properties,
		vk::Image& image,
		vk::DeviceMemory& imageMemory,
		vk::ImageCreateFlags flags,
		uint32_t arrayLayers
	)
	{
		vk::ImageCreateInfo imageInfo = {};
		imageInfo.setImageType(vk::ImageType::e2D);
		imageInfo.setExtent({ width, height, 1 });;
		imageInfo.setMipLevels(1);
		imageInfo.setArrayLayers(arrayLayers);
		imageInfo.setFormat(format);
		imageInfo.setTiling(tiling);
		imageInfo.setInitialLayout(vk::ImageLayout::eUndefined);
		imageInfo.setUsage(usage);
		imageInfo.setSamples(vk::SampleCountFlagBits::e1);
		imageInfo.setSharingMode(vk::SharingMode::eExclusive);
		imageInfo.setFlags(flags);

		// Create image
		if (m_logicalDevice.createImage(&imageInfo, nullptr, &image) != vk::Result::eSuccess)
			throwError("VULKAN: Unable to create image.");

		vk::MemoryRequirements memRequirements = m_logicalDevice.getImageMemoryRequirements(image);

		vk::MemoryAllocateInfo allocInfo = {};
		allocInfo.setAllocationSize(memRequirements.size);
		allocInfo.setMemoryTypeIndex(findMemoryType(memRequirements.memoryTypeBits, properties));

		// Allocate memory
		if (m_logicalDevice.allocateMemory(&allocInfo, nullptr, &imageMemory) != vk::Result::eSuccess)
			throwError("VULKAN: Unable to allocate image memory.");

		m_logicalDevice.bindImageMemory(image, imageMemory, 0);
	}

	void Graphics::transitionImageLayout(const vk::Image& image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, uint32_t imageCount)
	{
		vk::CommandBuffer commandBuffer = beginSingleTimeCommands();

		vk::ImageMemoryBarrier barrier = {};
		barrier.setOldLayout(oldLayout);
		barrier.setNewLayout(newLayout);
		barrier.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
		barrier.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
		barrier.setImage(image);

		// Stage masks
		vk::PipelineStageFlags dstStageFlags;
		vk::PipelineStageFlags srcStageFlags;

		if (newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
		{
			barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;

			if (format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint)
				barrier.subresourceRange.aspectMask |= vk::ImageAspectFlagBits::eStencil;
		}
		else
			barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;

		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = imageCount;

		if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal)
		{
			barrier.srcAccessMask = (vk::AccessFlagBits)0;
			barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

			srcStageFlags = vk::PipelineStageFlagBits::eTopOfPipe;
			dstStageFlags = vk::PipelineStageFlagBits::eTransfer;
		}
		else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
		{
			barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
			barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

			srcStageFlags = vk::PipelineStageFlagBits::eTransfer;
			dstStageFlags = vk::PipelineStageFlagBits::eFragmentShader | vk::PipelineStageFlagBits::eVertexInput;
		}
		else if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
		{
			barrier.srcAccessMask = (vk::AccessFlagBits)0;
			barrier.dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;

			srcStageFlags = vk::PipelineStageFlagBits::eTopOfPipe;
			dstStageFlags = vk::PipelineStageFlagBits::eEarlyFragmentTests;
		}
		else
			throwError("VULKAN: Unsupported layout transition");

		commandBuffer.pipelineBarrier
		(
			srcStageFlags, dstStageFlags,
			(vk::DependencyFlagBits)0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		endSingleTimeCommands(commandBuffer);
	}

	void Graphics::copyBufferToImage(const vk::Buffer& buffer, const vk::Image& image, uint32_t width, uint32_t height, uint32_t imageCount, uint32_t imageSize)
	{
		vk::CommandBuffer commandBuffer = beginSingleTimeCommands();

		std::vector<vk::BufferImageCopy> regions(imageCount);
		uint32_t offset = 0;

		for (size_t i = 0; i < regions.size(); ++i)
		{
			regions[i].setBufferOffset(offset);
			regions[i].setBufferRowLength(0);
			regions[i].setBufferImageHeight(0);
			regions[i].imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
			regions[i].imageSubresource.mipLevel = 0;
			regions[i].imageSubresource.baseArrayLayer = static_cast<uint32_t>(i);
			regions[i].imageSubresource.layerCount = 1;
			regions[i].setImageOffset({ 0, 0, 0 });
			regions[i].setImageExtent({ width, height, 1 });
			offset += imageSize;
		}

		// Copy buffer to image
		commandBuffer.copyBufferToImage
		(
			buffer,
			image,
			vk::ImageLayout::eTransferDstOptimal,
			static_cast<uint32_t>(regions.size()),
			regions.data()
		);

		endSingleTimeCommands(commandBuffer);
	}

	vk::ImageView Graphics::createImageView(const vk::Image& image, vk::Format format, vk::ImageAspectFlags aspectFlags, vk::ImageViewType viewType, uint32_t imageCount)
	{
		vk::ImageViewCreateInfo viewInfo = {};
		viewInfo.setImage(image);
		viewInfo.setViewType(viewType);
		viewInfo.setFormat(format);
		viewInfo.setComponents(vk::ComponentMapping());
		viewInfo.subresourceRange.aspectMask = aspectFlags;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = imageCount;

		vk::ImageView view;

		if (m_logicalDevice.createImageView(&viewInfo, nullptr, &view) != vk::Result::eSuccess)
			throwError("VULKAN: Unable to create image view.");

		return view;
	}

	size_t Graphics::getDeviceScore(vk::PhysicalDevice device)
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

	QueueFamilyIndices Graphics::findQueueFamilies(vk::PhysicalDevice device)
	{
		QueueFamilyIndices indices = {};

		// Get queue families
		const auto queueFamilies = device.getQueueFamilyProperties();

		// Find optimal queue families
		int i = 0;
		for (const auto& queueFamily : queueFamilies)
		{
			vk::Bool32 presentSupport = device.getSurfaceSupportKHR(i, m_surface);

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
			++i;
		}

		return indices;
	}

	bool Graphics::supportsDeviceExtensions(vk::PhysicalDevice physicalDevice)
	{
		// Extensions avaliable
		const auto availableExtensions = physicalDevice.enumerateDeviceExtensionProperties();

		std::set<std::string> requiredExtensions(m_deviceExtensions.begin(), m_deviceExtensions.end());

		for (const auto& extension : availableExtensions)
			requiredExtensions.erase(extension.extensionName);

		return requiredExtensions.empty();
	}

	std::vector<const char*> Graphics::getLayers(const std::vector<const char*>& layers)
	{
		// Get avaliable layers
		auto avaliableLayers = vk::enumerateInstanceLayerProperties();

		// Layers found
		std::vector<const char*> foundLayers = {};

		// Loop over every layer requested
		for (auto layerName : layers)
			// Loop over every polled layer
			for (const auto& layerProperties : avaliableLayers)
				// Check if it's the requested layer
				if (strcmp(layerName, layerProperties.layerName) == 0)
				{
					foundLayers.push_back(layerName);
					break;
				}

		return foundLayers;
	}

	std::vector<const char*> Graphics::getExtensions(const std::vector<const char*>& extensions)
	{
		// Get installed extensions
		auto installedExtensions = vk::enumerateInstanceExtensionProperties();

		// Requested extensions
		std::vector<const char*> foundExtensions = {};

		// Loop over every extension requested
		for (auto& extension : extensions)
			// Loop over every polled extension
			for (const auto& extensionProperty : installedExtensions)
				// Check if it's the requested extension
				if (strcmp(extension, extensionProperty.extensionName) == 0)
				{
					foundExtensions.push_back(extension);
					break;
				}

		return foundExtensions;
	}

	void Graphics::initSurfaceFormats(vk::PhysicalDevice physicalDevice)
	{
		// Check to see if we can display RGB colors
		const auto surfaceFormats = physicalDevice.getSurfaceFormatsKHR(m_surface);

		if (surfaceFormats.size() == 1 && surfaceFormats[0].format == vk::Format::eUndefined)
			m_colorFormat = vk::Format::eB8G8R8A8Unorm;
		else
			m_colorFormat = surfaceFormats[0].format;

		m_colorSpace = surfaceFormats[0].colorSpace;

		const auto formatProperties = physicalDevice.getFormatProperties(vk::Format::eR8G8B8A8Unorm);

		// Find a suitable depth format to use, starting with the best format
		std::vector<vk::Format> depthFormats =
		{
			vk::Format::eD32SfloatS8Uint,
			vk::Format::eD32Sfloat,
			vk::Format::eD24UnormS8Uint,
			vk::Format::eD16UnormS8Uint,
			vk::Format::eD16Unorm
		};

		for (const auto format : depthFormats)
		{
			const auto depthFormatProperties = physicalDevice.getFormatProperties(format);

			// Format must support depth stencil attachment for optimal tiling
			if (depthFormatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment)
			{
				m_depthFormat = format;
				break;
			}
		}
	}

	vk::CommandBuffer Graphics::beginSingleTimeCommands()
	{
		vk::CommandBufferAllocateInfo allocInfo = {};
		allocInfo.setLevel(vk::CommandBufferLevel::ePrimary);
		allocInfo.setCommandPool(m_singleUsePool);
		allocInfo.setCommandBufferCount(1);

		auto commandBuffer = m_logicalDevice.allocateCommandBuffers(allocInfo)[0];

		vk::CommandBufferBeginInfo beginInfo = {};
		beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

		commandBuffer.begin(beginInfo);

		return commandBuffer;
	}

	void Graphics::endSingleTimeCommands(vk::CommandBuffer commandBuffer)
	{
		commandBuffer.end();

		vk::SubmitInfo submitInfo = {};
		submitInfo.setCommandBufferCount(1);
		submitInfo.setPCommandBuffers(&commandBuffer);

		m_graphicsQueue.submit(1, &submitInfo, { nullptr });
		m_graphicsQueue.waitIdle();

		m_logicalDevice.freeCommandBuffers(m_singleUsePool, 1, &commandBuffer);
	}
}