#include <array>
#include "../Core/Debugging.hpp"
#include "Graphics.hpp"

namespace gust
{
	Graphics::Graphics(const std::string& name, uint32_t width, uint32_t height) :	
		m_name(name),
		m_height(height),
		m_width(width)
	{
		// Check parameters
		assert(width > 0);
		assert(height > 0);
		assert(name != "");

		m_width = width;
		m_height = height;

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
		for (size_t i = 0; i < extensionCount; i++)
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

		// Create surface manager
		m_surfaceManager = std::make_unique<VulkanSurfaceManager>(m_instance, m_window);

		// Create device manager
		m_deviceManager = std::make_unique<VulkanDeviceManager>(m_instance, m_surfaceManager.get(), m_layers, m_extensions);

		// Initialize surface formats
		m_surfaceManager->initSurfaceFormats(m_deviceManager->getPhysicalDevice());

		// Check if the physical device supports the surface.
		if (!m_deviceManager->getPhysicalDevice().getSurfaceSupportKHR(m_deviceManager->getQueueFamilyIndices().presentFamily, m_surfaceManager->getSurface()))
			throwError("VULKAN: Physical device does not support presenting to the surface.");

		// Create queue manager
		m_queueManager = std::make_unique<VulkanQueueManager>(m_deviceManager->getLogicalDevice(), m_deviceManager->getQueueFamilyIndices());

		// Create command manager
		m_commandManager = std::make_unique<VulkanCommandManager>(m_deviceManager.get(), m_queueManager.get());
	}

	Graphics::~Graphics()
	{
		m_deviceManager->getLogicalDevice().waitIdle();

		// Destroy command manager
		m_commandManager = nullptr;

		// Destroy queue manager
		m_queueManager = nullptr;

		// Destroy device manager
		m_deviceManager = nullptr;

		// Destroy surface manager
		m_surfaceManager = nullptr;

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
		m_deviceManager->getLogicalDevice().waitIdle();

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
			static_cast<uint32_t>(m_deviceManager->getQueueFamilyIndices().graphicsFamily),
			static_cast<uint32_t>(m_deviceManager->getQueueFamilyIndices().transferFamily)
		};

		vk::BufferCreateInfo bufferInfo = {};
		bufferInfo.setSize(size);
		bufferInfo.setUsage(usage);
		bufferInfo.setQueueFamilyIndexCount(2);
		bufferInfo.setPQueueFamilyIndices(queues.data());
		bufferInfo.setSharingMode(vk::SharingMode::eConcurrent);

		// Create buffer
		buffer.buffer = m_deviceManager->getLogicalDevice().createBuffer(bufferInfo);

		vk::MemoryRequirements memRequirements = {};
		m_deviceManager->getLogicalDevice().getBufferMemoryRequirements(buffer.buffer, &memRequirements);

		vk::MemoryAllocateInfo allocInfo = {};
		allocInfo.setAllocationSize(memRequirements.size);
		allocInfo.setMemoryTypeIndex(findMemoryType(memRequirements.memoryTypeBits, properties));

		// Allocate memory
		buffer.memory = m_deviceManager->getLogicalDevice().allocateMemory(allocInfo);

		// Bind memory
		m_deviceManager->getLogicalDevice().bindBufferMemory(buffer.buffer, buffer.memory, 0);

		return buffer;
	}

	void Graphics::copyBuffer(const vk::Buffer& sourceBuffer, const vk::Buffer& destinationBuffer, vk::DeviceSize size)
	{
		vk::CommandBufferAllocateInfo allocInfo = {};
		allocInfo.setLevel(vk::CommandBufferLevel::ePrimary);
		allocInfo.setCommandPool(m_commandManager->getTransferPool());
		allocInfo.setCommandBufferCount(1);

		// Create command buffer
		vk::CommandBuffer commandBuffer = {};
		commandBuffer = m_deviceManager->getLogicalDevice().allocateCommandBuffers(allocInfo)[0];

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
		m_queueManager->getTransferQueue().submit(1, &submitInfo, {});
		m_queueManager->getTransferQueue().waitIdle();

		// Destroy command buffer
		m_deviceManager->getLogicalDevice().freeCommandBuffers(m_commandManager->getTransferPool(), 1, &commandBuffer);
	}

	uint32_t Graphics::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties)
	{
		// Qeury memory properties
		auto memProperties = m_deviceManager->getPhysicalDevice().getMemoryProperties();

		// Find memory type
		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
				return i;

		throwError("VULKAN: Failed to find suitable memory type");
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
		vk::DeviceMemory& imageMemory
	)
	{
		vk::ImageCreateInfo imageInfo = {};
		imageInfo.setImageType(vk::ImageType::e2D);
		imageInfo.setExtent({ width, height, 1 });;
		imageInfo.setMipLevels(1);
		imageInfo.setArrayLayers(1);
		imageInfo.setFormat(format);
		imageInfo.setTiling(tiling);
		imageInfo.setInitialLayout(vk::ImageLayout::eUndefined);
		imageInfo.setUsage(usage);
		imageInfo.setSamples(vk::SampleCountFlagBits::e1);
		imageInfo.setSharingMode(vk::SharingMode::eExclusive);

		// Create image
		if (m_deviceManager->getLogicalDevice().createImage(&imageInfo, nullptr, &image) != vk::Result::eSuccess)
			throwError("VULKAN: Unable to create image.");

		vk::MemoryRequirements memRequirements = m_deviceManager->getLogicalDevice().getImageMemoryRequirements(image);

		vk::MemoryAllocateInfo allocInfo = {};
		allocInfo.setAllocationSize(memRequirements.size);
		allocInfo.setMemoryTypeIndex(findMemoryType(memRequirements.memoryTypeBits, properties));

		// Allocate memory
		if (m_deviceManager->getLogicalDevice().allocateMemory(&allocInfo, nullptr, &imageMemory) != vk::Result::eSuccess)
			throwError("VULKAN: Unable to allocate image memory.");

		m_deviceManager->getLogicalDevice().bindImageMemory(image, imageMemory, 0);
	}

	void Graphics::transitionImageLayout(const vk::Image& image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout)
	{
		vk::CommandBuffer commandBuffer = m_commandManager->beginSingleTimeCommands();

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
		barrier.subresourceRange.layerCount = 1;

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

		m_commandManager->endSingleTimeCommands(commandBuffer);
	}

	void Graphics::copyBufferToImage(const vk::Buffer& buffer, const vk::Image& image, uint32_t width, uint32_t height)
	{
		vk::CommandBuffer commandBuffer = m_commandManager->beginSingleTimeCommands();

		vk::BufferImageCopy region = {};
		region.setBufferOffset(0);
		region.setBufferRowLength(0);
		region.setBufferImageHeight(0);
		region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;
		region.setImageOffset({ 0, 0, 0 });
		region.setImageExtent({ width, height, 1 });

		// Copy buffer to image
		commandBuffer.copyBufferToImage
		(
			buffer,
			image,
			vk::ImageLayout::eTransferDstOptimal,
			1,
			&region
		);

		m_commandManager->endSingleTimeCommands(commandBuffer);
	}

	vk::ImageView Graphics::createImageView(const vk::Image& image, vk::Format format, vk::ImageAspectFlags aspectFlags)
	{
		vk::ImageViewCreateInfo viewInfo = {};
		viewInfo.setImage(image);
		viewInfo.setViewType(vk::ImageViewType::e2D);
		viewInfo.setFormat(format);
		viewInfo.subresourceRange.aspectMask = aspectFlags;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		vk::ImageView view;

		if (m_deviceManager->getLogicalDevice().createImageView(&viewInfo, nullptr, &view) != vk::Result::eSuccess)
			throwError("VULKAN: Unable to create image view.");

		return view;
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
}