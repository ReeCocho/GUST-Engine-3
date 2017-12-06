#include "../Core/Debugging.hpp"
#include "VulkanSwapchainManager.hpp"

// VulkanSwapchainManager
namespace gust
{
	VulkanSwapchainManager::VulkanSwapchainManager
	(
		Graphics* graphics,
		uint32_t width,
		uint32_t height,
		vk::RenderPass renderPass,
		QueueFamilyIndices qfi
	) :
		m_graphics(graphics),
		m_renderPass(renderPass),
		m_queueFamilyIndices(qfi),
		m_width(width),
		m_height(height)
	{
		initSwapchain();
		initDepthResources();
		initSwapchainBuffers();
	}

	VulkanSwapchainManager::~VulkanSwapchainManager()
	{
		auto logicalDevice = m_graphics->getLogicalDevice();

		// Cleanup depth texture
		logicalDevice.destroyImageView(m_depthTexture.imageView);
		logicalDevice.destroyImage(m_depthTexture.image);
		logicalDevice.freeMemory(m_depthTexture.memory);

		// Destroy framebuffers and views
		for (size_t i = 0; i < m_buffers.size(); i++)
		{
			logicalDevice.destroyFramebuffer(m_buffers[i].frameBuffer);
			logicalDevice.destroyImageView(m_buffers[i].views[0]);
		}

		// Cleanup swapchain
		logicalDevice.destroySwapchainKHR(m_swapchain);
	}

	void VulkanSwapchainManager::initSwapchain()
	{
		// Get surface capabilities, present modes, and size
		const auto surfaceCapabilities = m_graphics->getPhysicalDevice().getSurfaceCapabilitiesKHR(m_graphics->getSurface());
		const auto surfacePresentModes = m_graphics->getPhysicalDevice().getSurfacePresentModesKHR(m_graphics->getSurface());

		// Get best present mode
		auto presentMode = vk::PresentModeKHR::eImmediate;

		for (auto& pm : surfacePresentModes)
			if (pm == vk::PresentModeKHR::eMailbox)
			{
				presentMode = vk::PresentModeKHR::eMailbox;
				break;
			}

		// Make sure we have enough images
		assert(surfaceCapabilities.maxImageCount >= 3);

		// Get optimum image count
		uint32_t imageCount = surfaceCapabilities.minImageCount + 1;
		if (surfaceCapabilities.maxImageCount > 0 && imageCount > surfaceCapabilities.maxImageCount)
			imageCount = surfaceCapabilities.maxImageCount;

		vk::SwapchainCreateInfoKHR swapchainCreateInfo = {};
		swapchainCreateInfo.setSurface(m_graphics->getSurface());						// Surface to create swapchain for
		swapchainCreateInfo.setMinImageCount(imageCount);											// Minimum number of swapchain images
		swapchainCreateInfo.setImageFormat(m_graphics->getSurfaceColorFormat());		// Swapchain images color format
		swapchainCreateInfo.setImageColorSpace(m_graphics->getSurfaceColorSpace());	// Swapchain images color space
		swapchainCreateInfo.setImageExtent(vk::Extent2D(m_width, m_height));						// Size of swapchain images.
		swapchainCreateInfo.setImageArrayLayers(1);													// I honestly don't know what this does 
		swapchainCreateInfo.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment);				// Type of image
		swapchainCreateInfo.setPreTransform(surfaceCapabilities.currentTransform);					// Image transformation
		swapchainCreateInfo.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque);				// Alpha mode
		swapchainCreateInfo.setPresentMode(presentMode);											// Presentation mode
		swapchainCreateInfo.setClipped(true);														// We don't care about the color of obsucured pixels

		std::array<uint32_t, 2> qfi =
		{
			static_cast<uint32_t>(m_queueFamilyIndices.graphicsFamily),
			static_cast<uint32_t>(m_queueFamilyIndices.presentFamily)
		};

		if (qfi[0] != qfi[1])
		{
			swapchainCreateInfo.setQueueFamilyIndexCount(static_cast<uint32_t>(qfi.size()));	// Number of queue family indices
			swapchainCreateInfo.setPQueueFamilyIndices(qfi.data());								// Queue family indices
			swapchainCreateInfo.setImageSharingMode(vk::SharingMode::eConcurrent);				// Sharing mode for images
		}
		else
		{
			swapchainCreateInfo.setQueueFamilyIndexCount(0);
			swapchainCreateInfo.setPQueueFamilyIndices(nullptr);
			swapchainCreateInfo.setImageSharingMode(vk::SharingMode::eExclusive);
		}

		// Create swapchain
		if (m_graphics->getLogicalDevice().createSwapchainKHR(&swapchainCreateInfo, nullptr, &m_swapchain) != vk::Result::eSuccess)
			throwError("VULKAN: Unable to create swapchain.");

		// Get swapchain images
		m_images = m_graphics->getLogicalDevice().getSwapchainImagesKHR(m_swapchain);
	}

	void VulkanSwapchainManager::initDepthResources()
	{
		// Create depth image data
		m_graphics->createImage
		(
			m_width,
			m_height,
			m_graphics->getDepthFormat(),
			vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eDepthStencilAttachment,
			vk::MemoryPropertyFlagBits::eDeviceLocal,
			m_depthTexture.image,
			m_depthTexture.memory
		);

		// Create image view
		m_depthTexture.imageView = m_graphics->createImageView
		(
			m_depthTexture.image,
			m_graphics->getDepthFormat(),
			vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil
		);

		// Transition layout
		m_graphics->transitionImageLayout
		(
			m_depthTexture.image,
			m_graphics->getDepthFormat(),
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::eDepthStencilAttachmentOptimal
		);
	}

	void VulkanSwapchainManager::initSwapchainBuffers()
	{
		auto logicalDevice = m_graphics->getLogicalDevice();
		m_buffers.resize(m_images.size());

		for (size_t i = 0; i < m_images.size(); i++)
		{
			m_buffers[i].image = m_images[i];

			vk::ImageViewCreateInfo imageView = {};
			imageView.setFlags(vk::ImageViewCreateFlags());										// Creation flags
			imageView.setImage(m_images[i]);													// Image to view
			imageView.setViewType(vk::ImageViewType::e2D);										// Type of image
			imageView.setFormat(m_graphics->getSurfaceColorFormat());				// Color format of the image
			imageView.setComponents(vk::ComponentMapping());									// I don't know
			imageView.setSubresourceRange({ vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 });		// Resource range

			// Color
			m_buffers[i].views[0] = logicalDevice.createImageView(imageView);

			// Depth
			m_buffers[i].views[1] = m_depthTexture.imageView;

			vk::FramebufferCreateInfo framebuffer = {};
			framebuffer.setFlags(vk::FramebufferCreateFlags());									// Creation flags
			framebuffer.setRenderPass(m_renderPass);											// Render pass
			framebuffer.setAttachmentCount(static_cast<uint32_t>(m_buffers[i].views.size()));	// Image view count
			framebuffer.setPAttachments(m_buffers[i].views.data());								// Image views
			framebuffer.setWidth(m_width);														// Width
			framebuffer.setHeight(m_height);													// Height
			framebuffer.setLayers(1);															// Layer count

			// Framebuffer
			m_buffers[i].frameBuffer = logicalDevice.createFramebuffer(framebuffer);
		}
	}
}