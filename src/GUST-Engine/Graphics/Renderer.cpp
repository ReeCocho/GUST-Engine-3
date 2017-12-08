#include "Renderer.hpp"

namespace gust
{
	void Renderer::startup(Graphics* graphics, size_t threadCount)
	{
		m_graphics = graphics;
		m_threadPool = std::make_unique<ThreadPool>(threadCount);

		initRenderPass();
		initSwapchain();
		initDepthResources();
		initSwapchainBuffers();
	}

	void Renderer::shutdown()
	{
		m_threadPool = nullptr;

		auto logicalDevice = m_graphics->getLogicalDevice();

		// Cleanup
		logicalDevice.destroyRenderPass(m_renderPass);
		// logicalDevice.destroySemaphore(m_imageAvailableSemaphore);
		// logicalDevice.destroySemaphore(m_renderFinishedSemaphore);

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

	void Renderer::render()
	{
		
	}

	void Renderer::initRenderPass()
	{
		// Describes color attachment usage
		vk::AttachmentDescription colorAttachment = {};
		colorAttachment.setFormat(m_graphics->getSurfaceColorFormat());
		colorAttachment.setSamples(vk::SampleCountFlagBits::e1);
		colorAttachment.setLoadOp(vk::AttachmentLoadOp::eClear);
		colorAttachment.setStoreOp(vk::AttachmentStoreOp::eStore);
		colorAttachment.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
		colorAttachment.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
		colorAttachment.setInitialLayout(vk::ImageLayout::eUndefined);
		colorAttachment.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

		// Describes depth attachment usage
		vk::AttachmentDescription depthAttachment = {};
		depthAttachment.setFormat(m_graphics->getDepthFormat());
		depthAttachment.setSamples(vk::SampleCountFlagBits::e1);
		depthAttachment.setLoadOp(vk::AttachmentLoadOp::eClear);
		depthAttachment.setStoreOp(vk::AttachmentStoreOp::eDontCare);
		depthAttachment.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
		depthAttachment.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
		depthAttachment.setInitialLayout(vk::ImageLayout::eUndefined);
		depthAttachment.setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

		vk::AttachmentReference colorAttachmentRef = {};
		colorAttachmentRef.setAttachment(0);
		colorAttachmentRef.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

		vk::AttachmentReference depthAttachmentRef = {};
		depthAttachmentRef.setAttachment(1);
		depthAttachmentRef.setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

		vk::SubpassDescription subpass = {};
		subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);
		subpass.setColorAttachmentCount(1);
		subpass.setPColorAttachments(&colorAttachmentRef);
		subpass.setPDepthStencilAttachment(&depthAttachmentRef);

		vk::SubpassDependency dependency = {};
		dependency.setSrcSubpass(VK_SUBPASS_EXTERNAL);
		dependency.setDstSubpass(0);
		dependency.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
		dependency.setSrcAccessMask(vk::AccessFlagBits::eMemoryRead);
		dependency.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
		dependency.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite);

		std::array<vk::AttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };

		vk::RenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.setAttachmentCount(static_cast<uint32_t>(attachments.size()));
		renderPassInfo.setPAttachments(attachments.data());
		renderPassInfo.setSubpassCount(1);
		renderPassInfo.setPSubpasses(&subpass);
		renderPassInfo.setDependencyCount(1);
		renderPassInfo.setPDependencies(&dependency);

		if (m_graphics->getLogicalDevice().createRenderPass(&renderPassInfo, nullptr, &m_renderPass) != vk::Result::eSuccess)
			throwError("VULKAN: Failed to create render pass");
	}

	void Renderer::initSwapchain()
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
		swapchainCreateInfo.setSurface(m_graphics->getSurface());									// Surface to create swapchain for
		swapchainCreateInfo.setMinImageCount(imageCount);											// Minimum number of swapchain images
		swapchainCreateInfo.setImageFormat(m_graphics->getSurfaceColorFormat());					// Swapchain images color format
		swapchainCreateInfo.setImageColorSpace(m_graphics->getSurfaceColorSpace());					// Swapchain images color space
		swapchainCreateInfo.setImageExtent({ m_graphics->getWidth(), m_graphics->getHeight() });	// Size of swapchain images.
		swapchainCreateInfo.setImageArrayLayers(1);													// I honestly don't know what this does 
		swapchainCreateInfo.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment);				// Type of image
		swapchainCreateInfo.setPreTransform(surfaceCapabilities.currentTransform);					// Image transformation
		swapchainCreateInfo.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque);				// Alpha mode
		swapchainCreateInfo.setPresentMode(presentMode);											// Presentation mode
		swapchainCreateInfo.setClipped(true);														// We don't care about the color of obsucured pixels

		std::array<uint32_t, 2> qfi =
		{
			static_cast<uint32_t>(m_graphics->getQueueFamilyIndices().graphicsFamily),
			static_cast<uint32_t>(m_graphics->getQueueFamilyIndices().presentFamily)
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

	void Renderer::initDepthResources()
	{
		// Create depth image data
		m_graphics->createImage
		(
			m_graphics->getWidth(),
			m_graphics->getHeight(),
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

	void Renderer::initSwapchainBuffers()
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
			imageView.setFormat(m_graphics->getSurfaceColorFormat());							// Color format of the image
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
			framebuffer.setWidth(m_graphics->getWidth());										// Width
			framebuffer.setHeight(m_graphics->getHeight());										// Height
			framebuffer.setLayers(1);															// Layer count

			// Framebuffer
			m_buffers[i].frameBuffer = logicalDevice.createFramebuffer(framebuffer);
		}
	}
}