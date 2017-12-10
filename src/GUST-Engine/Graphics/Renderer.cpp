#include "Renderer.hpp"

namespace gust
{
	void Renderer::startup(Graphics* graphics, size_t threadCount)
	{
		m_graphics = graphics;
		m_threadPool = std::make_unique<ThreadPool>(threadCount);

		initRenderPasses();
		initSwapchain();
		initDepthResources();
		initSwapchainBuffers();
		initSemaphores();
		initCommandBuffers();
	}

	void Renderer::shutdown()
	{
		m_threadPool = nullptr;

		auto logicalDevice = m_graphics->getLogicalDevice();

		// Cleanup
		m_graphics->destroyCommandBuffer(m_primaryCommandBuffer);

		logicalDevice.destroyRenderPass(m_renderPasses.onscreen);
		logicalDevice.destroyRenderPass(m_renderPasses.offscreen);
		logicalDevice.destroyRenderPass(m_renderPasses.lighting);
		
		logicalDevice.destroySemaphore(m_semaphores.imageAvailable);
		logicalDevice.destroySemaphore(m_semaphores.renderFinished);
		logicalDevice.destroySemaphore(m_semaphores.offscreen);

		// Cleanup depth texture
		logicalDevice.destroyImageView(m_depthTexture.imageView);
		logicalDevice.destroyImage(m_depthTexture.image);
		logicalDevice.freeMemory(m_depthTexture.memory);

		// Destroy framebuffers and views
		for (auto& buffer : m_swapchain.buffers)
		{
			logicalDevice.destroyFramebuffer(buffer.frameBuffer);
			logicalDevice.destroyImageView(buffer.view);
		}

		// Cleanup swapchain
		logicalDevice.destroySwapchainKHR(m_swapchain.swapchain);
	}

	void Renderer::render()
	{
		// Get image to present
		uint32_t imageIndex = m_graphics->getLogicalDevice().acquireNextImageKHR
		(
			m_swapchain.swapchain,
			std::numeric_limits<uint64_t>::max(),
			m_semaphores.imageAvailable,
			{ nullptr }
		).value;

		// Render
		{
			vk::CommandBufferBeginInfo beginInfo = {};
			beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse);
			beginInfo.setPInheritanceInfo(nullptr);

			std::array<vk::ClearValue, 2> clearValues = {};
			clearValues[0].setColor(std::array<float, 4>{ 0.0f, 0.0f, 0.0f, 0.0f });
			clearValues[1].setColor(std::array<float, 4>{ 0.0f, 0.0f, 0.0f, 0.0f });

			vk::RenderPassBeginInfo renderPassInfo = {};
			renderPassInfo.setRenderPass(m_renderPasses.onscreen);
			renderPassInfo.setFramebuffer(m_swapchain.buffers[imageIndex].frameBuffer);
			renderPassInfo.renderArea.setOffset({ 0, 0 });
			renderPassInfo.renderArea.setExtent(vk::Extent2D(m_graphics->getWidth(), m_graphics->getHeight()));
			renderPassInfo.setClearValueCount(static_cast<uint32_t>(clearValues.size()));
			renderPassInfo.setPClearValues(clearValues.data());

			m_primaryCommandBuffer.begin(beginInfo);
			m_primaryCommandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

			// Set viewport
			vk::Viewport viewport = {};
			viewport.setHeight((float)m_graphics->getHeight());
			viewport.setWidth((float)m_graphics->getWidth());
			m_primaryCommandBuffer.setViewport(0, 1, &viewport);

			// Set scissor
			vk::Rect2D scissor = {};
			scissor.setExtent({ m_graphics->getWidth(), m_graphics->getHeight() });
			scissor.setOffset({ 0, 0 });
			m_primaryCommandBuffer.setScissor(0, 1, &scissor);

			m_primaryCommandBuffer.endRenderPass();
			m_primaryCommandBuffer.end();
		}

		std::array<vk::Semaphore, 1> semaphores =
		{
			m_semaphores.imageAvailable
			// m_offscreen
		};

		std::array<vk::PipelineStageFlags, 1> waitStages =
		{
			vk::PipelineStageFlagBits::eColorAttachmentOutput
			// vk::PipelineStageFlagBits::eColorAttachmentOutput
		};

		vk::SubmitInfo submitInfo = {};
		submitInfo.setWaitSemaphoreCount(static_cast<uint32_t>(semaphores.size()));
		submitInfo.setPWaitSemaphores(semaphores.data());
		submitInfo.setPWaitDstStageMask(waitStages.data());
		submitInfo.setCommandBufferCount(1);
		submitInfo.setPCommandBuffers(&m_primaryCommandBuffer);
		submitInfo.setSignalSemaphoreCount(1);
		submitInfo.setPSignalSemaphores(&m_semaphores.renderFinished);

		// Submit draw command
		m_graphics->getGraphicsQueue().submit(1, &submitInfo, { nullptr });

		vk::PresentInfoKHR presentInfo = {};
		presentInfo.setWaitSemaphoreCount(1);
		presentInfo.setPWaitSemaphores(&m_semaphores.renderFinished);
		presentInfo.setSwapchainCount(1);
		presentInfo.setPSwapchains(&m_swapchain.swapchain);
		presentInfo.setPImageIndices(&imageIndex);
		presentInfo.setPResults(nullptr);

		// Present and wait
		m_graphics->getPresentationQueue().presentKHR(presentInfo);
		m_graphics->getLogicalDevice().waitIdle();

		// // Reset graphics command buffers
		// m_commandManager.resetGraphicsCommandBuffers();
	}

	void Renderer::initRenderPasses()
	{
		// Onscreen
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

			vk::AttachmentReference colorAttachmentRef = {};
			colorAttachmentRef.setAttachment(0);
			colorAttachmentRef.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

			vk::SubpassDescription subpass = {};
			subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);
			subpass.setColorAttachmentCount(1);
			subpass.setPColorAttachments(&colorAttachmentRef);

			vk::SubpassDependency dependency = {};
			dependency.setSrcSubpass(VK_SUBPASS_EXTERNAL);
			dependency.setDstSubpass(0);
			dependency.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
			dependency.setSrcAccessMask(vk::AccessFlagBits::eMemoryRead);
			dependency.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
			dependency.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite);
			dependency.setDependencyFlags(vk::DependencyFlagBits::eByRegion);

			vk::RenderPassCreateInfo renderPassInfo = {};
			renderPassInfo.setAttachmentCount(1);
			renderPassInfo.setPAttachments(&colorAttachment);
			renderPassInfo.setSubpassCount(1);
			renderPassInfo.setPSubpasses(&subpass);
			renderPassInfo.setDependencyCount(1);
			renderPassInfo.setPDependencies(&dependency);

			if (m_graphics->getLogicalDevice().createRenderPass(&renderPassInfo, nullptr, &m_renderPasses.onscreen) != vk::Result::eSuccess)
				throwError("VULKAN: Failed to create render pass");
		}

		// Offscreen
		{
			// Set up separate renderpass with references to the color and depth attachments
			std::array<vk::AttachmentDescription, 4> attachmentDescs = {};

			// Input attachment properties
			for (size_t i = 0; i < 4; i++)
			{
				attachmentDescs[i].setSamples(vk::SampleCountFlagBits::e1);
				attachmentDescs[i].setLoadOp(vk::AttachmentLoadOp::eClear);
				attachmentDescs[i].setStoreOp(vk::AttachmentStoreOp::eStore);
				attachmentDescs[i].setStencilLoadOp(vk::AttachmentLoadOp::eClear);
				attachmentDescs[i].setStencilStoreOp(vk::AttachmentStoreOp::eStore);

				if (i == 3)
				{
					attachmentDescs[i].setInitialLayout(vk::ImageLayout::eUndefined);
					attachmentDescs[i].setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
				}
				else
				{
					attachmentDescs[i].setInitialLayout(vk::ImageLayout::eUndefined);
					attachmentDescs[i].setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal);
				}
			}

			// Formats
			attachmentDescs[0].setFormat(vk::Format::eR16G16B16A16Sfloat);
			attachmentDescs[1].setFormat(vk::Format::eR16G16B16A16Sfloat);
			attachmentDescs[2].setFormat(m_graphics->getSurfaceColorFormat());
			attachmentDescs[3].setFormat(m_graphics->getDepthFormat());

			std::array<vk::AttachmentReference, 3> colorReferences = {};
			colorReferences[0] = { 0, vk::ImageLayout::eColorAttachmentOptimal };
			colorReferences[1] = { 1, vk::ImageLayout::eColorAttachmentOptimal };
			colorReferences[2] = { 2, vk::ImageLayout::eColorAttachmentOptimal };

			vk::AttachmentReference depthReference = {};
			depthReference.setAttachment(3);
			depthReference.setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

			vk::SubpassDescription subpass = {};
			subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);
			subpass.setPColorAttachments(colorReferences.data());
			subpass.setColorAttachmentCount(static_cast<uint32_t>(colorReferences.size()));
			subpass.setPDepthStencilAttachment(&depthReference);

			// Use subpass dependencies for attachment layput transitions
			std::array<vk::SubpassDependency, 2> dependencies =
			{
				vk::SubpassDependency
				(
					VK_SUBPASS_EXTERNAL,
					0,
					vk::PipelineStageFlagBits::eBottomOfPipe,
					vk::PipelineStageFlagBits::eColorAttachmentOutput,
					vk::AccessFlagBits::eMemoryRead,
					vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite,
					vk::DependencyFlagBits::eByRegion
				),
				vk::SubpassDependency
				(
					0,
					VK_SUBPASS_EXTERNAL,
					vk::PipelineStageFlagBits::eColorAttachmentOutput,
					vk::PipelineStageFlagBits::eBottomOfPipe,
					vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite,
					vk::AccessFlagBits::eMemoryRead,
					vk::DependencyFlagBits::eByRegion
				)
			};

			vk::RenderPassCreateInfo renderPassInfo = {};
			renderPassInfo.setPAttachments(attachmentDescs.data());
			renderPassInfo.setAttachmentCount(static_cast<uint32_t>(attachmentDescs.size()));
			renderPassInfo.setSubpassCount(1);
			renderPassInfo.setPSubpasses(&subpass);
			renderPassInfo.setDependencyCount(static_cast<uint32_t>(dependencies.size()));
			renderPassInfo.setPDependencies(dependencies.data());

			// Create render pass
			m_renderPasses.offscreen = m_graphics->getLogicalDevice().createRenderPass(renderPassInfo);
		}

		// Lighting
		{
			// Set up separate renderpass with references to the color and depth attachments
			std::array<vk::AttachmentDescription, 4> attachmentDescs = {};

			// Input attachment properties
			for (size_t i = 0; i < 4; i++)
			{
				attachmentDescs[i].setSamples(vk::SampleCountFlagBits::e1);
				attachmentDescs[i].setLoadOp(vk::AttachmentLoadOp::eLoad);
				attachmentDescs[i].setStoreOp(vk::AttachmentStoreOp::eStore);
				attachmentDescs[i].setStencilLoadOp(vk::AttachmentLoadOp::eLoad);
				attachmentDescs[i].setStencilStoreOp(vk::AttachmentStoreOp::eStore);

				if (i == 3)
				{
					attachmentDescs[i].setInitialLayout(vk::ImageLayout::eUndefined);
					attachmentDescs[i].setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
				}
				else
				{
					attachmentDescs[i].setInitialLayout(vk::ImageLayout::eUndefined);
					attachmentDescs[i].setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal);
				}
			}

			// Formats
			attachmentDescs[0].setFormat(vk::Format::eR16G16B16A16Sfloat);
			attachmentDescs[1].setFormat(vk::Format::eR16G16B16A16Sfloat);
			attachmentDescs[2].setFormat(m_graphics->getSurfaceColorFormat());
			attachmentDescs[3].setFormat(m_graphics->getDepthFormat());

			std::array<vk::AttachmentReference, 3> colorReferences = {};
			colorReferences[0] = { 0, vk::ImageLayout::eColorAttachmentOptimal };
			colorReferences[1] = { 1, vk::ImageLayout::eColorAttachmentOptimal };
			colorReferences[2] = { 2, vk::ImageLayout::eColorAttachmentOptimal };

			vk::AttachmentReference depthReference = {};
			depthReference.setAttachment(3);
			depthReference.setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

			vk::SubpassDescription subpass = {};
			subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);
			subpass.setPColorAttachments(colorReferences.data());
			subpass.setColorAttachmentCount(static_cast<uint32_t>(colorReferences.size()));
			subpass.setPDepthStencilAttachment(&depthReference);

			// Use subpass dependencies for attachment layput transitions
			std::array<vk::SubpassDependency, 2> dependencies =
			{
				vk::SubpassDependency
				(
					VK_SUBPASS_EXTERNAL,
					0,
					vk::PipelineStageFlagBits::eBottomOfPipe,
					vk::PipelineStageFlagBits::eColorAttachmentOutput,
					vk::AccessFlagBits::eMemoryRead,
					vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite,
					vk::DependencyFlagBits::eByRegion
				),
				vk::SubpassDependency
				(
					0,
					VK_SUBPASS_EXTERNAL,
					vk::PipelineStageFlagBits::eColorAttachmentOutput,
					vk::PipelineStageFlagBits::eBottomOfPipe,
					vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite,
					vk::AccessFlagBits::eMemoryRead,
					vk::DependencyFlagBits::eByRegion
				)
			};

			vk::RenderPassCreateInfo renderPassInfo = {};
			renderPassInfo.setPAttachments(attachmentDescs.data());
			renderPassInfo.setAttachmentCount(static_cast<uint32_t>(attachmentDescs.size()));
			renderPassInfo.setSubpassCount(1);
			renderPassInfo.setPSubpasses(&subpass);
			renderPassInfo.setDependencyCount(static_cast<uint32_t>(dependencies.size()));
			renderPassInfo.setPDependencies(dependencies.data());

			// Create render pass
			m_renderPasses.lighting = m_graphics->getLogicalDevice().createRenderPass(renderPassInfo);
		}
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
		if (m_graphics->getLogicalDevice().createSwapchainKHR(&swapchainCreateInfo, nullptr, &m_swapchain.swapchain) != vk::Result::eSuccess)
			throwError("VULKAN: Unable to create swapchain.");

		// Get swapchain images
		m_swapchain.images = m_graphics->getLogicalDevice().getSwapchainImagesKHR(m_swapchain.swapchain);
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
		m_swapchain.buffers.resize(m_swapchain.images.size());

		for (size_t i = 0; i < m_swapchain.images.size(); i++)
		{
			m_swapchain.buffers[i].image = m_swapchain.images[i];

			vk::ImageViewCreateInfo imageView = {};
			imageView.setFlags(vk::ImageViewCreateFlags());										// Creation flags
			imageView.setImage(m_swapchain.images[i]);											// Image to view
			imageView.setViewType(vk::ImageViewType::e2D);										// Type of image
			imageView.setFormat(m_graphics->getSurfaceColorFormat());							// Color format of the image
			imageView.setComponents(vk::ComponentMapping());									// I don't know
			imageView.setSubresourceRange({ vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 });		// Resource range

			// Color
			m_swapchain.buffers[i].view = logicalDevice.createImageView(imageView);

			vk::FramebufferCreateInfo framebuffer = {};
			framebuffer.setFlags(vk::FramebufferCreateFlags());				// Creation flags
			framebuffer.setRenderPass(m_renderPasses.onscreen);				// Render pass
			framebuffer.setAttachmentCount(1);								// Image view count
			framebuffer.setPAttachments(&m_swapchain.buffers[i].view);		// Image views
			framebuffer.setWidth(m_graphics->getWidth());					// Width
			framebuffer.setHeight(m_graphics->getHeight());					// Height
			framebuffer.setLayers(1);										// Layer count

			// Framebuffer
			m_swapchain.buffers[i].frameBuffer = logicalDevice.createFramebuffer(framebuffer);
		}
	}

	void Renderer::initSemaphores()
	{
		vk::SemaphoreCreateInfo semaphoreInfo = {};

		m_semaphores.imageAvailable = m_graphics->getLogicalDevice().createSemaphore(semaphoreInfo);
		m_semaphores.renderFinished = m_graphics->getLogicalDevice().createSemaphore(semaphoreInfo);
		m_semaphores.offscreen = m_graphics->getLogicalDevice().createSemaphore(semaphoreInfo);
	}

	void Renderer::initCommandBuffers()
	{
		m_primaryCommandBuffer = m_graphics->createCommandBuffer(vk::CommandBufferLevel::ePrimary);
	}
}