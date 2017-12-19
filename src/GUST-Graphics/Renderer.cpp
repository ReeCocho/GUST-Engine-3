#include <FileIO.hpp>
#include "Renderer.hpp"

namespace gust
{
	void Renderer::startup(Graphics* graphics, size_t threadCount)
	{
		m_graphics = graphics;
		m_threadPool = std::make_unique<ThreadPool>(threadCount);
		m_cameraAllocator = std::make_unique<ResourceAllocator<VirtualCamera>>(10, 4);

		initCommandPools();
		initRenderPasses();
		initSwapchain();
		initDepthResources();
		initSwapchainBuffers();
		initSemaphores();
		initCommandBuffers();
		initLighting();
		initDescriptorSetLayouts();
		initDescriptorPool();
		initDescriptorSets();
		initShaders();
	}

	void Renderer::shutdown()
	{
		auto logicalDevice = m_graphics->getLogicalDevice();

		// Destroy thread pool
		m_threadPool = nullptr;

		// Destroy cameras
		for(size_t i = 0; i < m_cameraAllocator->getMaxResourceCount(); ++i)
			if (m_cameraAllocator->isAllocated(i))
			{
				VirtualCamera* camera = m_cameraAllocator->getResourceByHandle(i);
				logicalDevice.destroyFramebuffer(camera->frameBuffer);
				destroyCommandBuffer(camera->commandBuffer);
				destroyCommandBuffer(camera->lightingCommandBuffer);
			}

		// Destroy cameras
		m_cameraAllocator = nullptr;

		// Destroy screen quad
		m_screenQuad = nullptr;

		// Cleanup
		logicalDevice.destroyShaderModule(m_lightingShader.vertexShader);
		logicalDevice.destroyShaderModule(m_lightingShader.fragmentShader);
		logicalDevice.destroyDescriptorSetLayout(m_lightingShader.textureDescriptorSetLayout);
		logicalDevice.destroyPipelineLayout(m_lightingShader.graphicsPipelineLayout);
		logicalDevice.destroyPipeline(m_lightingShader.graphicsPipeline);

		logicalDevice.destroyShaderModule(m_screenShader.vertexShader);
		logicalDevice.destroyShaderModule(m_screenShader.fragmentShader);
		logicalDevice.destroyDescriptorSetLayout(m_screenShader.textureDescriptorSetLayout);
		logicalDevice.destroyPipelineLayout(m_screenShader.graphicsPipelineLayout);
		logicalDevice.destroyPipeline(m_screenShader.graphicsPipeline);

		logicalDevice.destroyBuffer(m_lightingUniformBuffer.buffer);
		logicalDevice.freeMemory(m_lightingUniformBuffer.memory);

		logicalDevice.destroyDescriptorPool(m_descriptors.descriptorPool);

		logicalDevice.destroyDescriptorSetLayout(m_descriptors.descriptorSetLayout);
		logicalDevice.destroyDescriptorSetLayout(m_descriptors.screenDescriptorSetLayout);
		logicalDevice.destroyDescriptorSetLayout(m_descriptors.lightingDescriptorSetLayout);

		destroyCommandBuffer(m_commands.primaryCommandBuffer);

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

		// Destroy command pools
		for (auto pool : m_commands.pools)
			logicalDevice.destroyCommandPool(pool);

		// Cleanup swapchain
		logicalDevice.destroySwapchainKHR(m_swapchain.swapchain);
	}

	void Renderer::render()
	{
		// Submit lighting data
		submitLightingData();

		// Draw everything to every camera
		bool drew = false;

		for (size_t i = 0; i < m_cameraAllocator->getMaxResourceCount(); ++i)
			if (m_cameraAllocator->isAllocated(i))
			{
				drawToCamera(Handle<VirtualCamera>(m_cameraAllocator.get(), i));
				drew = true;
			}

		// Return early if we didn't draw anything (No need to present again)
		if (!drew)
			return;

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
			auto commandBuffer = m_commands.primaryCommandBuffer.buffer;

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

			commandBuffer.begin(beginInfo);
			commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

			// Set viewport
			vk::Viewport viewport = {};
			viewport.setHeight(static_cast<float>(m_graphics->getHeight()));
			viewport.setWidth(static_cast<float>(m_graphics->getWidth()));
			commandBuffer.setViewport(0, 1, &viewport);

			// Set scissor
			vk::Rect2D scissor = {};
			scissor.setExtent({ m_graphics->getWidth(), m_graphics->getHeight() });
			scissor.setOffset({ 0, 0 });
			commandBuffer.setScissor(0, 1, &scissor);

			// If the main camera is valid draw it's framebuffer
			if (m_mainCamera.getResourceAllocator() && m_mainCamera.get())
			{
				// Bind descriptor sets
				commandBuffer.bindDescriptorSets
				(
					vk::PipelineBindPoint::eGraphics,
					m_screenShader.graphicsPipelineLayout,
					0,
					1,
					&m_descriptors.screenDescriptorSet,
					0,
					nullptr
				);

				// Bind graphics pipeline
				commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_screenShader.graphicsPipeline);

				// Bind vertex and index buffer
				vk::Buffer vertexBuffer = m_screenQuad->getVertexUniformBuffer().buffer;
				vk::DeviceSize offset = 0;
				commandBuffer.bindVertexBuffers(0, 1, &vertexBuffer, &offset);
				commandBuffer.bindIndexBuffer(m_screenQuad->getIndexUniformBuffer().buffer, 0, vk::IndexType::eUint32);

				// Draw
				commandBuffer.drawIndexed(static_cast<uint32_t>(m_screenQuad->getIndexCount()), 1, 0, 0, 0);
			}

			commandBuffer.endRenderPass();
			commandBuffer.end();
		}

		std::array<vk::Semaphore, 2> semaphores =
		{
			m_semaphores.imageAvailable,
			m_semaphores.offscreen
		};

		std::array<vk::PipelineStageFlags, 2> waitStages =
		{
			vk::PipelineStageFlagBits::eColorAttachmentOutput,
			vk::PipelineStageFlagBits::eColorAttachmentOutput
		};

		vk::SubmitInfo submitInfo = {};
		submitInfo.setWaitSemaphoreCount(static_cast<uint32_t>(semaphores.size()));
		submitInfo.setPWaitSemaphores(semaphores.data());
		submitInfo.setPWaitDstStageMask(waitStages.data());
		submitInfo.setCommandBufferCount(1);
		submitInfo.setPCommandBuffers(&m_commands.primaryCommandBuffer.buffer);
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
		m_graphics->getPresentationQueue().waitIdle();

		// Clear mesh queue
		m_meshes.clear();
	}

	Handle<VirtualCamera> Renderer::createCamera()
	{
		// Resize the array if necessary
		if (m_cameraAllocator->getResourceCount() == m_cameraAllocator->getMaxResourceCount())
			m_cameraAllocator->resize(m_cameraAllocator->getMaxResourceCount() + 10, true);

		// Allocate camera and call constructor
		auto camera = Handle<VirtualCamera>(m_cameraAllocator.get(), m_cameraAllocator->allocate());
		::new(camera.get())(VirtualCamera)();

		camera->width = m_graphics->getWidth();
		camera->height = m_graphics->getHeight();

		camera->commandBuffer = createCommandBuffer(vk::CommandBufferLevel::ePrimary);
		camera->lightingCommandBuffer = createCommandBuffer(vk::CommandBufferLevel::ePrimary);

		// Color attachments

		// (World space) Positions
		FrameBufferAttachment position = createAttachment
		(
			vk::Format::eR16G16B16A16Sfloat,
			vk::ImageUsageFlagBits::eColorAttachment
		);

		// (World space) Normals
		FrameBufferAttachment normals = createAttachment
		(
			vk::Format::eR16G16B16A16Sfloat,
			vk::ImageUsageFlagBits::eColorAttachment
		);

		// Albedo (color)
		FrameBufferAttachment color = createAttachment
		(
			m_graphics->getSurfaceColorFormat(),
			vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc
		);

		// Depth attachment
		FrameBufferAttachment depth = createAttachment
		(
			m_graphics->getDepthFormat(),
			vk::ImageUsageFlagBits::eDepthStencilAttachment
		);

		// Create sampler to sample from the attachments
		vk::SamplerCreateInfo sampler = {};
		sampler.setMagFilter(vk::Filter::eNearest);
		sampler.setMinFilter(vk::Filter::eNearest);
		sampler.setMipmapMode(vk::SamplerMipmapMode::eLinear);
		sampler.setAddressModeU(vk::SamplerAddressMode::eClampToEdge);
		sampler.setAddressModeV(sampler.addressModeU);
		sampler.setAddressModeW(sampler.addressModeU);
		sampler.setMipLodBias(0.0f);
		sampler.setMaxAnisotropy(1.0f);
		sampler.setMinLod(0.0f);
		sampler.setMaxLod(1.0f);
		sampler.setBorderColor(vk::BorderColor::eFloatOpaqueWhite);

		// Create samplers
		vk::Sampler positionSampler = m_graphics->getLogicalDevice().createSampler(sampler);
		vk::Sampler normalSampler = m_graphics->getLogicalDevice().createSampler(sampler);
		vk::Sampler colorSampler = m_graphics->getLogicalDevice().createSampler(sampler);
		vk::Sampler depthSampler = m_graphics->getLogicalDevice().createSampler(sampler);

		// Set attachments
		camera->position	= std::make_unique<Texture>(m_graphics, position.image, position.view, positionSampler, position.memory, camera->width, camera->height);
		camera->normal		= std::make_unique<Texture>(m_graphics, normals.image, normals.view, normalSampler, normals.memory, camera->width, camera->height);
		camera->color		= std::make_unique<Texture>(m_graphics, color.image, color.view, colorSampler, color.memory, camera->width, camera->height);
		camera->depth		= std::make_unique<Texture>(m_graphics, depth.image, depth.view, depthSampler, depth.memory, camera->width, camera->height);

		std::array<vk::ImageView, 4> attachments;
		attachments[0] = camera->position->getImageView();
		attachments[1] = camera->normal->getImageView();
		attachments[2] = camera->color->getImageView();
		attachments[3] = camera->depth->getImageView();

		vk::FramebufferCreateInfo fbufCreateInfo = {};
		fbufCreateInfo.setPNext(nullptr);
		fbufCreateInfo.setRenderPass(m_renderPasses.offscreen);
		fbufCreateInfo.setPAttachments(attachments.data());
		fbufCreateInfo.setAttachmentCount(static_cast<uint32_t>(attachments.size()));
		fbufCreateInfo.setWidth(camera->width);
		fbufCreateInfo.setHeight(camera->height);
		fbufCreateInfo.setLayers(1);

		// Create framebuffer
		camera->frameBuffer = m_graphics->getLogicalDevice().createFramebuffer(fbufCreateInfo);

		return camera;
	}

	void Renderer::initCommandPools()
	{
		// Create a command pools
		auto commandPoolInfo = vk::CommandPoolCreateInfo
		(
			vk::CommandPoolCreateFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer),
			m_graphics->getQueueFamilyIndices().graphicsFamily
		);

		m_commands.pools.resize(m_threadPool->getWorkerCount());
		for (size_t i = 0; i < m_commands.pools.size(); ++i)
			m_commands.pools[i] = m_graphics->getLogicalDevice().createCommandPool(commandPoolInfo);
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
			for (size_t i = 0; i < 4; ++i)
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
			for (size_t i = 0; i < 4; ++i)
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

		for (size_t i = 0; i < m_swapchain.images.size(); ++i)
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
		m_commands.primaryCommandBuffer = createCommandBuffer(vk::CommandBufferLevel::ePrimary);
	}

	void Renderer::initLighting()
	{
		// create lighting uniform buffer
		m_lightingUniformBuffer = m_graphics->createBuffer
		(
			static_cast<vk::DeviceSize>(sizeof(LightingData)),
			vk::BufferUsageFlagBits::eUniformBuffer,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
		);
	}

	void Renderer::initDescriptorSetLayouts()
	{
		// Standard material descriptor set layout
		{
			std::array<vk::DescriptorSetLayoutBinding, 4> bindings = {};

			// Vertex bindings
			bindings[0].setBinding(0);
			bindings[0].setDescriptorType(vk::DescriptorType::eUniformBuffer);
			bindings[0].setDescriptorCount(1);
			bindings[0].setStageFlags(vk::ShaderStageFlagBits::eVertex);

			bindings[1].setBinding(1);
			bindings[1].setDescriptorType(vk::DescriptorType::eUniformBuffer);
			bindings[1].setDescriptorCount(1);
			bindings[1].setStageFlags(vk::ShaderStageFlagBits::eVertex);

			// Fragment bindings
			bindings[2].setBinding(2);
			bindings[2].setDescriptorType(vk::DescriptorType::eUniformBuffer);
			bindings[2].setDescriptorCount(1);
			bindings[2].setStageFlags(vk::ShaderStageFlagBits::eFragment);

			bindings[3].setBinding(3);
			bindings[3].setDescriptorType(vk::DescriptorType::eUniformBuffer);
			bindings[3].setDescriptorCount(1);
			bindings[3].setStageFlags(vk::ShaderStageFlagBits::eFragment);

			vk::DescriptorSetLayoutCreateInfo createInfo = {};
			createInfo.setBindingCount(static_cast<uint32_t>(bindings.size()));
			createInfo.setPBindings(bindings.data());

			// Create descriptor set layout
			m_descriptors.descriptorSetLayout = m_graphics->getLogicalDevice().createDescriptorSetLayout(createInfo);
		}

		// Lighting descriptor set Layout
		{
			std::array<vk::DescriptorSetLayoutBinding, 4> bindings = {};

			// Lighting
			bindings[0].setBinding(0);
			bindings[0].setDescriptorType(vk::DescriptorType::eUniformBuffer);
			bindings[0].setDescriptorCount(1);
			bindings[0].setStageFlags(vk::ShaderStageFlagBits::eFragment);

			// Position binding
			bindings[1].setBinding(1);
			bindings[1].setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
			bindings[1].setDescriptorCount(1);
			bindings[1].setStageFlags(vk::ShaderStageFlagBits::eFragment);

			// Normal binding
			bindings[2].setBinding(2);
			bindings[2].setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
			bindings[2].setDescriptorCount(1);
			bindings[2].setStageFlags(vk::ShaderStageFlagBits::eFragment);

			// Color binding
			bindings[3].setBinding(3);
			bindings[3].setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
			bindings[3].setDescriptorCount(1);
			bindings[3].setStageFlags(vk::ShaderStageFlagBits::eFragment);

			vk::DescriptorSetLayoutCreateInfo createInfo = {};
			createInfo.setBindingCount(static_cast<uint32_t>(bindings.size()));
			createInfo.setPBindings(bindings.data());

			// Create descriptor set layout
			m_descriptors.lightingDescriptorSetLayout = m_graphics->getLogicalDevice().createDescriptorSetLayout(createInfo);
		}

		// Screen descriptor set Layout
		{
			std::array<vk::DescriptorSetLayoutBinding, 1> bindings = {};

			// Color binding
			bindings[0].setBinding(0);
			bindings[0].setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
			bindings[0].setDescriptorCount(1);
			bindings[0].setStageFlags(vk::ShaderStageFlagBits::eFragment);

			vk::DescriptorSetLayoutCreateInfo createInfo = {};
			createInfo.setBindingCount(static_cast<uint32_t>(bindings.size()));
			createInfo.setPBindings(bindings.data());

			// Create descriptor set layout
			m_descriptors.screenDescriptorSetLayout = m_graphics->getLogicalDevice().createDescriptorSetLayout(createInfo);
		}
	}

	void Renderer::initDescriptorPool()
	{
		std::array<vk::DescriptorPoolSize, 2> poolSizes = {};
		poolSizes[0].setDescriptorCount(2);
		poolSizes[0].setType(vk::DescriptorType::eUniformBuffer);

		poolSizes[1].setDescriptorCount(4);
		poolSizes[1].setType(vk::DescriptorType::eCombinedImageSampler);

		vk::DescriptorPoolCreateInfo poolInfo = {};
		poolInfo.setPoolSizeCount(static_cast<uint32_t>(poolSizes.size()));
		poolInfo.setPPoolSizes(poolSizes.data());
		poolInfo.setMaxSets(2);

		m_descriptors.descriptorPool = m_graphics->getLogicalDevice().createDescriptorPool(poolInfo);
	}

	void Renderer::initDescriptorSets()
	{
		// Lighting descriptor set
		{
			vk::DescriptorSetAllocateInfo allocInfo = {};
			allocInfo.setDescriptorPool(m_descriptors.descriptorPool);
			allocInfo.setDescriptorSetCount(1);
			allocInfo.setPSetLayouts(&m_descriptors.lightingDescriptorSetLayout);

			// Allocate descriptor sets
			m_descriptors.lightingDescriptorSet = m_graphics->getLogicalDevice().allocateDescriptorSets(allocInfo)[0];

			// Lighting
			vk::DescriptorBufferInfo bufferInfo = {};
			bufferInfo.setBuffer(m_lightingUniformBuffer.buffer);
			bufferInfo.setOffset(0);
			bufferInfo.setRange(static_cast<VkDeviceSize>(sizeof(LightingData)));

			vk::WriteDescriptorSet descWrite = {};
			descWrite.setDstSet(m_descriptors.lightingDescriptorSet);
			descWrite.setDstBinding(0);
			descWrite.setDstArrayElement(0);
			descWrite.setDescriptorType(vk::DescriptorType::eUniformBuffer);
			descWrite.setDescriptorCount(1);
			descWrite.setPBufferInfo(&bufferInfo);
			descWrite.setPImageInfo(nullptr);
			descWrite.setPTexelBufferView(nullptr);

			m_graphics->getLogicalDevice().updateDescriptorSets(1, &descWrite, 0, nullptr);
		}

		// Screen descriptor set
		{
			vk::DescriptorSetAllocateInfo allocInfo = {};
			allocInfo.setDescriptorPool(m_descriptors.descriptorPool);
			allocInfo.setDescriptorSetCount(1);
			allocInfo.setPSetLayouts(&m_descriptors.screenDescriptorSetLayout);

			// Allocate descriptor sets
			m_descriptors.screenDescriptorSet = m_graphics->getLogicalDevice().allocateDescriptorSets(allocInfo)[0];
		}
	}

	void Renderer::initShaders()
	{
		std::vector<uint32_t> inds = 
		{
			0, 2, 1,
			2, 3, 1
		};

		std::vector<glm::vec3> verts =
		{
			glm::vec3(-1, -1, 0),
			glm::vec3(-1,  1, 0),
			glm::vec3( 1, -1, 0),
			glm::vec3( 1,  1, 0)
		};

		std::vector<glm::vec2> uvs =
		{
			glm::vec2(0, 0),
			glm::vec2(0, 1),
			glm::vec2(1, 0),
			glm::vec2(1, 1)
		};

		// Make quad
		m_screenQuad = std::make_unique<Mesh>
		(
			m_graphics,
			inds,
			verts,
			uvs
		);

		// Create screen shader
		{
			// Load shader byte code
			std::vector<char> fragSource = readBinary(GUST_SCREEN_FRAGMENT_SHADER_PATH);
			std::vector<char> vertSource = readBinary(GUST_SCREEN_VERTEX_SHADER_PATH);

			// Vertex shader module
			{
				// Align code
				std::vector<uint32_t> codeAligned(vertSource.size() / sizeof(uint32_t) + 1);
				memcpy(codeAligned.data(), vertSource.data(), vertSource.size());

				vk::ShaderModuleCreateInfo createInfo = {};
				createInfo.setCodeSize(vertSource.size());
				createInfo.setPCode(codeAligned.data());

				// Create shader module
				m_screenShader.vertexShader = m_graphics->getLogicalDevice().createShaderModule(createInfo);
			}

			// Fragment shader module
			{
				// Align code
				std::vector<uint32_t> codeAligned(fragSource.size() / sizeof(uint32_t) + 1);
				memcpy(codeAligned.data(), fragSource.data(), fragSource.size());

				vk::ShaderModuleCreateInfo createInfo = {};
				createInfo.setCodeSize(fragSource.size());
				createInfo.setPCode(codeAligned.data());

				// Create shader module
				m_screenShader.fragmentShader = m_graphics->getLogicalDevice().createShaderModule(createInfo);
			}

			// Vertex shader stage create info
			vk::PipelineShaderStageCreateInfo vertShaderStageInfo = {};
			vertShaderStageInfo.setStage(vk::ShaderStageFlagBits::eVertex);
			vertShaderStageInfo.setModule(m_screenShader.vertexShader);
			vertShaderStageInfo.setPName("main");

			// Fragment shader stage create info
			vk::PipelineShaderStageCreateInfo fragShaderStageInfo = {};
			fragShaderStageInfo.setStage(vk::ShaderStageFlagBits::eFragment);
			fragShaderStageInfo.setModule(m_screenShader.fragmentShader);
			fragShaderStageInfo.setPName("main");

			m_screenShader.shaderStages = { vertShaderStageInfo, fragShaderStageInfo };
		}

		// Create lighting shader
		{
			// Load shader byte code
			std::vector<char> fragSource = readBinary(GUST_LIGHTING_FRAGMENT_SHADER_PATH);
			std::vector<char> vertSource = readBinary(GUST_LIGHTING_VERTEX_SHADER_PATH);

			// Vertex shader module
			{
				// Align code
				std::vector<uint32_t> codeAligned(vertSource.size() / sizeof(uint32_t) + 1);
				memcpy(codeAligned.data(), vertSource.data(), vertSource.size());

				vk::ShaderModuleCreateInfo createInfo = {};
				createInfo.setCodeSize(vertSource.size());
				createInfo.setPCode(codeAligned.data());

				// Create shader module
				m_lightingShader.vertexShader = m_graphics->getLogicalDevice().createShaderModule(createInfo);
			}

			// Fragment shader module
			{
				// Align code
				std::vector<uint32_t> codeAligned(fragSource.size() / sizeof(uint32_t) + 1);
				memcpy(codeAligned.data(), fragSource.data(), fragSource.size());

				vk::ShaderModuleCreateInfo createInfo = {};
				createInfo.setCodeSize(fragSource.size());
				createInfo.setPCode(codeAligned.data());

				// Create shader module
				m_lightingShader.fragmentShader = m_graphics->getLogicalDevice().createShaderModule(createInfo);
			}

			// Vertex shader stage create info
			vk::PipelineShaderStageCreateInfo vertShaderStageInfo = {};
			vertShaderStageInfo.setStage(vk::ShaderStageFlagBits::eVertex);
			vertShaderStageInfo.setModule(m_lightingShader.vertexShader);
			vertShaderStageInfo.setPName("main");

			// Fragment shader stage create info
			vk::PipelineShaderStageCreateInfo fragShaderStageInfo = {};
			fragShaderStageInfo.setStage(vk::ShaderStageFlagBits::eFragment);
			fragShaderStageInfo.setModule(m_lightingShader.fragmentShader);
			fragShaderStageInfo.setPName("main");

			m_lightingShader.shaderStages = { vertShaderStageInfo, fragShaderStageInfo };
		}

		// Create lighting graphics pipeline
		{
			auto bindingDescription = Vertex::getBindingDescription();
			auto attributeDescriptions = Vertex::getAttributeDescriptions();

			vk::PipelineVertexInputStateCreateInfo vertexInputInfo = {};
			vertexInputInfo.setVertexBindingDescriptionCount(1);
			vertexInputInfo.setVertexAttributeDescriptionCount(static_cast<uint32_t>(attributeDescriptions.size()));
			vertexInputInfo.setPVertexBindingDescriptions(&bindingDescription);
			vertexInputInfo.setPVertexAttributeDescriptions(attributeDescriptions.data());

			vk::PipelineInputAssemblyStateCreateInfo inputAssembly = {};
			inputAssembly.setTopology(vk::PrimitiveTopology::eTriangleList);
			inputAssembly.setPrimitiveRestartEnable(false);

			vk::Viewport viewport = {};
			viewport.setX(0.0f);
			viewport.setY(0.0f);
			viewport.setWidth((float)m_graphics->getWidth());
			viewport.setHeight((float)m_graphics->getHeight());
			viewport.setMinDepth(0.0f);
			viewport.setMaxDepth(1.0f);

			vk::Extent2D extents = {};
			extents.setHeight(m_graphics->getHeight());
			extents.setWidth(m_graphics->getWidth());

			vk::Rect2D scissor = {};
			scissor.setOffset({ 0, 0 });
			scissor.setExtent(extents);

			vk::PipelineViewportStateCreateInfo viewportState = {};
			viewportState.setViewportCount(1);
			viewportState.setPViewports(&viewport);
			viewportState.setScissorCount(1);
			viewportState.setPScissors(&scissor);

			vk::PipelineRasterizationStateCreateInfo rasterizer = {};
			rasterizer.setDepthClampEnable(false);
			rasterizer.setRasterizerDiscardEnable(false);
			rasterizer.setPolygonMode(vk::PolygonMode::eFill);
			rasterizer.setLineWidth(1.0f);
			rasterizer.setCullMode(vk::CullModeFlagBits::eBack);
			rasterizer.setFrontFace(vk::FrontFace::eClockwise);
			rasterizer.setDepthBiasEnable(false);
			rasterizer.setDepthBiasConstantFactor(0.0f);
			rasterizer.setDepthBiasClamp(0.0f);
			rasterizer.setDepthBiasSlopeFactor(0.0f);

			vk::PipelineMultisampleStateCreateInfo multisampling = {};
			multisampling.setSampleShadingEnable(false);
			multisampling.setRasterizationSamples(vk::SampleCountFlagBits::e1);
			multisampling.setMinSampleShading(1.0f);
			multisampling.setPSampleMask(nullptr);
			multisampling.setAlphaToCoverageEnable(false);
			multisampling.setAlphaToOneEnable(false);

			vk::StencilOpState stencil = {};
			stencil.setFailOp(vk::StencilOp::eKeep);
			stencil.setPassOp(vk::StencilOp::eReplace);
			stencil.setDepthFailOp(vk::StencilOp::eKeep);
			stencil.setCompareOp(vk::CompareOp::eEqual);
			stencil.setWriteMask(1);
			stencil.setReference(1);
			stencil.setCompareMask(1);

			vk::PipelineDepthStencilStateCreateInfo depthStencil = {};
			depthStencil.setDepthTestEnable(false);
			depthStencil.setDepthWriteEnable(false);
			depthStencil.setDepthCompareOp(vk::CompareOp::eLess);
			depthStencil.setDepthBoundsTestEnable(false);
			depthStencil.setMinDepthBounds(0.0f);
			depthStencil.setMaxDepthBounds(1.0f);
			depthStencil.setStencilTestEnable(true);
			depthStencil.setFront(stencil);
			depthStencil.setBack(stencil);

			std::array<vk::PipelineColorBlendAttachmentState, 3> colorBlendAttachments = {};

			for (size_t i = 0; i < 3; ++i)
			{
				colorBlendAttachments[i].setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);
				colorBlendAttachments[i].setBlendEnable(true);
				colorBlendAttachments[i].setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha);
				colorBlendAttachments[i].setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha);
				colorBlendAttachments[i].setColorBlendOp(vk::BlendOp::eAdd);
				colorBlendAttachments[i].setSrcAlphaBlendFactor(vk::BlendFactor::eOne);
				colorBlendAttachments[i].setDstAlphaBlendFactor(vk::BlendFactor::eZero);
				colorBlendAttachments[i].setAlphaBlendOp(vk::BlendOp::eAdd);
			}

			vk::PipelineColorBlendStateCreateInfo colorBlending = {};
			colorBlending.setLogicOpEnable(false);
			colorBlending.setLogicOp(vk::LogicOp::eCopy);
			colorBlending.setAttachmentCount(static_cast<uint32_t>(colorBlendAttachments.size()));
			colorBlending.setPAttachments(colorBlendAttachments.data());
			colorBlending.setBlendConstants({ 0.0f, 0.0f, 0.0f, 0.0f });

			vk::DynamicState dynamicStates[] =
			{
				vk::DynamicState::eViewport,
				vk::DynamicState::eLineWidth
			};

			vk::PipelineDynamicStateCreateInfo dynamicState = {};
			dynamicState.setDynamicStateCount(2);
			dynamicState.setPDynamicStates(dynamicStates);

			vk::PipelineLayoutCreateInfo pipelineLayoutInfo = {};
			pipelineLayoutInfo.setSetLayoutCount(1);
			pipelineLayoutInfo.setPSetLayouts(&m_descriptors.lightingDescriptorSetLayout);

			// Create pipeline layout
			m_lightingShader.graphicsPipelineLayout = m_graphics->getLogicalDevice().createPipelineLayout(pipelineLayoutInfo);

			vk::GraphicsPipelineCreateInfo pipelineInfo = {};
			pipelineInfo.setStageCount(2);
			pipelineInfo.setPStages(m_lightingShader.shaderStages.data());
			pipelineInfo.setPVertexInputState(&vertexInputInfo);
			pipelineInfo.setPInputAssemblyState(&inputAssembly);
			pipelineInfo.setPViewportState(&viewportState);
			pipelineInfo.setPRasterizationState(&rasterizer);
			pipelineInfo.setPMultisampleState(&multisampling);
			pipelineInfo.setPDepthStencilState(nullptr);
			pipelineInfo.setPColorBlendState(&colorBlending);
			pipelineInfo.setPDynamicState(nullptr);
			pipelineInfo.setLayout(m_lightingShader.graphicsPipelineLayout);
			pipelineInfo.setRenderPass(m_renderPasses.offscreen);
			pipelineInfo.setSubpass(0);
			pipelineInfo.setBasePipelineHandle({ nullptr });
			pipelineInfo.setBasePipelineIndex(-1);
			pipelineInfo.setPDepthStencilState(&depthStencil);

			// Create graphics pipeline
			m_lightingShader.graphicsPipeline = m_graphics->getLogicalDevice().createGraphicsPipeline({}, pipelineInfo);
		}

		// Create screen graphics pipeline
		{
			auto bindingDescription = Vertex::getBindingDescription();
			auto attributeDescriptions = Vertex::getAttributeDescriptions();

			vk::PipelineVertexInputStateCreateInfo vertexInputInfo = {};
			vertexInputInfo.setVertexBindingDescriptionCount(1);
			vertexInputInfo.setVertexAttributeDescriptionCount(static_cast<uint32_t>(attributeDescriptions.size()));
			vertexInputInfo.setPVertexBindingDescriptions(&bindingDescription);
			vertexInputInfo.setPVertexAttributeDescriptions(attributeDescriptions.data());

			vk::PipelineInputAssemblyStateCreateInfo inputAssembly = {};
			inputAssembly.setTopology(vk::PrimitiveTopology::eTriangleList);
			inputAssembly.setPrimitiveRestartEnable(false);

			vk::Viewport viewport = {};
			viewport.setX(0.0f);
			viewport.setY(0.0f);
			viewport.setWidth((float)m_graphics->getWidth());
			viewport.setHeight((float)m_graphics->getHeight());
			viewport.setMinDepth(0.0f);
			viewport.setMaxDepth(1.0f);

			vk::Extent2D extents = {};
			extents.setHeight(m_graphics->getHeight());
			extents.setWidth(m_graphics->getWidth());

			vk::Rect2D scissor = {};
			scissor.setOffset({ 0, 0 });
			scissor.setExtent(extents);

			vk::PipelineViewportStateCreateInfo viewportState = {};
			viewportState.setViewportCount(1);
			viewportState.setPViewports(&viewport);
			viewportState.setScissorCount(1);
			viewportState.setPScissors(&scissor);

			vk::PipelineRasterizationStateCreateInfo rasterizer = {};
			rasterizer.setDepthClampEnable(false);
			rasterizer.setRasterizerDiscardEnable(false);
			rasterizer.setPolygonMode(vk::PolygonMode::eFill);
			rasterizer.setLineWidth(1.0f);
			rasterizer.setCullMode(vk::CullModeFlagBits::eBack);
			rasterizer.setFrontFace(vk::FrontFace::eClockwise);
			rasterizer.setDepthBiasEnable(false);
			rasterizer.setDepthBiasConstantFactor(0.0f);
			rasterizer.setDepthBiasClamp(0.0f);
			rasterizer.setDepthBiasSlopeFactor(0.0f);

			vk::PipelineMultisampleStateCreateInfo multisampling = {};
			multisampling.setSampleShadingEnable(false);
			multisampling.setRasterizationSamples(vk::SampleCountFlagBits::e1);
			multisampling.setMinSampleShading(1.0f);
			multisampling.setPSampleMask(nullptr);
			multisampling.setAlphaToCoverageEnable(false);
			multisampling.setAlphaToOneEnable(false);

			vk::PipelineDepthStencilStateCreateInfo depthStencil = {};
			depthStencil.setDepthTestEnable(false);
			depthStencil.setDepthWriteEnable(false);
			depthStencil.setDepthCompareOp(vk::CompareOp::eLess);
			depthStencil.setDepthBoundsTestEnable(false);
			depthStencil.setMinDepthBounds(0.0f);
			depthStencil.setMaxDepthBounds(1.0f);
			depthStencil.setStencilTestEnable(false);
			depthStencil.setFront({});
			depthStencil.setBack({});

			vk::PipelineColorBlendAttachmentState colorBlendAttachment = {};
			colorBlendAttachment.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);
			colorBlendAttachment.setBlendEnable(false);
			colorBlendAttachment.setSrcColorBlendFactor(vk::BlendFactor::eOne);
			colorBlendAttachment.setDstColorBlendFactor(vk::BlendFactor::eZero);
			colorBlendAttachment.setColorBlendOp(vk::BlendOp::eAdd);
			colorBlendAttachment.setSrcAlphaBlendFactor(vk::BlendFactor::eOne);
			colorBlendAttachment.setDstAlphaBlendFactor(vk::BlendFactor::eZero);
			colorBlendAttachment.setAlphaBlendOp(vk::BlendOp::eAdd);

			vk::PipelineColorBlendStateCreateInfo colorBlending = {};
			colorBlending.setLogicOpEnable(false);
			colorBlending.setLogicOp(vk::LogicOp::eCopy);
			colorBlending.setAttachmentCount(1);
			colorBlending.setPAttachments(&colorBlendAttachment);
			colorBlending.setBlendConstants({ 0.0f, 0.0f, 0.0f, 0.0f });

			vk::DynamicState dynamicStates[] =
			{
				vk::DynamicState::eViewport,
				vk::DynamicState::eLineWidth
			};

			vk::PipelineDynamicStateCreateInfo dynamicState = {};
			dynamicState.setDynamicStateCount(2);
			dynamicState.setPDynamicStates(dynamicStates);

			vk::PipelineLayoutCreateInfo pipelineLayoutInfo = {};
			pipelineLayoutInfo.setSetLayoutCount(1);
			pipelineLayoutInfo.setPSetLayouts(&m_descriptors.screenDescriptorSetLayout);

			// Create pipeline layout
			m_screenShader.graphicsPipelineLayout = m_graphics->getLogicalDevice().createPipelineLayout(pipelineLayoutInfo);

			vk::GraphicsPipelineCreateInfo pipelineInfo = {};
			pipelineInfo.setStageCount(2);
			pipelineInfo.setPStages(m_screenShader.shaderStages.data());
			pipelineInfo.setPVertexInputState(&vertexInputInfo);
			pipelineInfo.setPInputAssemblyState(&inputAssembly);
			pipelineInfo.setPViewportState(&viewportState);
			pipelineInfo.setPRasterizationState(&rasterizer);
			pipelineInfo.setPMultisampleState(&multisampling);
			pipelineInfo.setPDepthStencilState(nullptr);
			pipelineInfo.setPColorBlendState(&colorBlending);
			pipelineInfo.setPDynamicState(nullptr);
			pipelineInfo.setLayout(m_screenShader.graphicsPipelineLayout);
			pipelineInfo.setRenderPass(m_renderPasses.onscreen);
			pipelineInfo.setSubpass(0);
			pipelineInfo.setBasePipelineHandle({ nullptr });
			pipelineInfo.setBasePipelineIndex(-1);
			pipelineInfo.setPDepthStencilState(&depthStencil);

			// Create graphics pipeline
			m_screenShader.graphicsPipeline = m_graphics->getLogicalDevice().createGraphicsPipeline({}, pipelineInfo);
		}
	}

	FrameBufferAttachment Renderer::createAttachment(vk::Format format, vk::ImageUsageFlags usage)
	{
		FrameBufferAttachment newAttachment = {};

		vk::ImageAspectFlags aspectMask = (vk::ImageAspectFlagBits)0;
		vk::ImageLayout imageLayout;

		newAttachment.format = format;

		// Get aspect mask and image layout
		if (usage & vk::ImageUsageFlagBits::eColorAttachment)
		{
			aspectMask = vk::ImageAspectFlagBits::eColor;
			imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
		}
		if (usage & vk::ImageUsageFlagBits::eDepthStencilAttachment)
		{
			aspectMask = vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;
			imageLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
		}

		vk::ImageCreateInfo image = {};
		image.setImageType(vk::ImageType::e2D);
		image.setFormat(format);
		image.extent.width = m_graphics->getWidth();
		image.extent.height = m_graphics->getHeight();
		image.extent.depth = 1;
		image.setMipLevels(1);
		image.setArrayLayers(1);
		image.setSamples(vk::SampleCountFlagBits::e1);
		image.setTiling(vk::ImageTiling::eOptimal);
		image.setUsage(usage | vk::ImageUsageFlagBits::eSampled);

		// Create image
		newAttachment.image = m_graphics->getLogicalDevice().createImage(image);

		vk::MemoryAllocateInfo memAlloc = {};
		vk::MemoryRequirements memReqs = m_graphics->getLogicalDevice().getImageMemoryRequirements(newAttachment.image);

		memAlloc.allocationSize = memReqs.size;
		memAlloc.memoryTypeIndex = m_graphics->findMemoryType(memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);

		// Allocate and bind image memory
		newAttachment.memory = m_graphics->getLogicalDevice().allocateMemory(memAlloc);
		m_graphics->getLogicalDevice().bindImageMemory(newAttachment.image, newAttachment.memory, 0);

		newAttachment.view = m_graphics->createImageView(newAttachment.image, format, aspectMask);

		return newAttachment;
	}

	void Renderer::submitLightingData()
	{
		// Set camera position
		if(m_mainCamera != Handle<VirtualCamera>::nullHandle())
			m_lightingData.viewPosition = { m_mainCamera->viewPosition, 1 };

		// Set light counts
		m_lightingData.directionalLightCount = static_cast<uint32_t>(m_directionalLights.size());
		m_lightingData.pointLightCount = static_cast<uint32_t>(m_pointLights.size());
		m_lightingData.spotLightCount = static_cast<uint32_t>(m_spotLights.size());

		// Set point lights
		for (size_t i = 0; i < m_lightingData.pointLightCount; ++i)
		{
			m_lightingData.pointLights[i] = m_pointLights.front();
			m_pointLights.pop();
		}

		// Set directional lights
		for (size_t i = 0; i < m_lightingData.directionalLightCount; ++i)
		{
			m_lightingData.directionalLights[i] = m_directionalLights.front();
			m_directionalLights.pop();
		}

		// Set spot lights
		for (size_t i = 0; i < m_lightingData.spotLightCount; ++i)
		{
			m_lightingData.spotLights[i] = m_spotLights.front();
			m_spotLights.pop();
		}

		// Set lighting data
		{
			void* cpyData;

			m_graphics->getLogicalDevice().mapMemory
			(
				m_lightingUniformBuffer.memory,
				0,
				static_cast<vk::DeviceSize>(sizeof(LightingData)),
				(vk::MemoryMapFlagBits)0,
				&cpyData
			);

			memcpy(cpyData, &m_lightingData, sizeof(LightingData));
			m_graphics->getLogicalDevice().unmapMemory(m_lightingUniformBuffer.memory);
		}
	}

	void Renderer::drawMeshToFramebuffer
	(
		const MeshData& mesh,
		const vk::CommandBufferInheritanceInfo& inheritanceInfo,
		size_t threadIndex,
		Handle<VirtualCamera> camera
	)
	{
		vk::CommandBufferBeginInfo beginInfo = {};
		beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eRenderPassContinue | vk::CommandBufferUsageFlagBits::eSimultaneousUse);
		beginInfo.setPInheritanceInfo(&inheritanceInfo);

		// Submit vertex data
		{
			VertexShaderData vData = {};
			vData.model = mesh.model;
			vData.MVP = camera->projection * camera->view * mesh.model;

			void* cpyData;

			m_graphics->getLogicalDevice().mapMemory
			(
				mesh.vertexUniformBuffer.memory,
				0,
				sizeof(VertexShaderData),
				(vk::MemoryMapFlagBits)0,
				&cpyData
			);

			memcpy(cpyData, &vData, sizeof(VertexShaderData));
			m_graphics->getLogicalDevice().unmapMemory(mesh.vertexUniformBuffer.memory);
		}

		// Submit fragment data
		{
			FragmentShaderData fData = {};
			fData.viewPosition = glm::vec4(camera->viewPosition, 1);

			void* cpyData;

			m_graphics->getLogicalDevice().mapMemory
			(
				mesh.fragmentUniformBuffer.memory,
				0,
				sizeof(FragmentShaderData),
				(vk::MemoryMapFlagBits)0,
				&cpyData
			);

			memcpy(cpyData, &fData, sizeof(FragmentShaderData));
			m_graphics->getLogicalDevice().unmapMemory(mesh.fragmentUniformBuffer.memory);
		}

		mesh.commandBuffer.buffer.begin(beginInfo);

		// Set viewport
		vk::Viewport viewport = {};
		viewport.setHeight((float)m_graphics->getHeight());
		viewport.setWidth((float)m_graphics->getWidth());
		viewport.setMinDepth(0);
		viewport.setMaxDepth(1);
		mesh.commandBuffer.buffer.setViewport(0, 1, &viewport);

		// Set scissor
		vk::Rect2D scissor = {};
		scissor.setExtent({ m_graphics->getWidth(), m_graphics->getHeight() });
		scissor.setOffset({ 0, 0 });
		mesh.commandBuffer.buffer.setScissor(0, 1, &scissor);

		// Bind graphics pipeline
		mesh.commandBuffer.buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, mesh.material->getShader()->getGraphicsPipeline());

		// Bind descriptor sets
		mesh.commandBuffer.buffer.bindDescriptorSets
		(
			vk::PipelineBindPoint::eGraphics,
			mesh.material->getShader()->getGraphicsPipelineLayout(),
			0,
			static_cast<uint32_t>(mesh.descriptorSets.size()),
			mesh.descriptorSets.data(),
			0,
			nullptr
		);

		// Bind vertex and index buffer
		vk::Buffer vertexBuffer = mesh.mesh->getVertexUniformBuffer().buffer;
		vk::DeviceSize offset = 0;
		mesh.commandBuffer.buffer.bindVertexBuffers(0, 1, &vertexBuffer, &offset);
		mesh.commandBuffer.buffer.bindIndexBuffer(mesh.mesh->getIndexUniformBuffer().buffer, 0, vk::IndexType::eUint32);

		// Draw
		mesh.commandBuffer.buffer.drawIndexed(static_cast<uint32_t>(mesh.mesh->getIndexCount()), 1, 0, 0, 0);
		mesh.commandBuffer.buffer.end();
	}

	void Renderer::drawToCamera(Handle<VirtualCamera> camera)
	{
		vk::CommandBufferBeginInfo cmdBufInfo = {};
		cmdBufInfo.setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse);
		cmdBufInfo.setPInheritanceInfo(nullptr);

		// Begin renderpass
		camera->commandBuffer.buffer.begin(cmdBufInfo);

		// Clear values for all attachments written in the fragment shader
		std::array<vk::ClearValue, 4> clearValues;
		clearValues[0].setColor(std::array<float, 4>{ 0.0f, 0.0f, 0.0f, 0.0f });
		clearValues[1].setColor(std::array<float, 4>{ 0.0f, 0.0f, 0.0f, 0.0f });
		clearValues[2].setColor(std::array<float, 4>{ 0.0f, 0.0f, 0.0f, 0.0f });
		clearValues[3].setDepthStencil({ 1.0f, 0 });

		vk::RenderPassBeginInfo renderPassBeginInfo = {};
		renderPassBeginInfo.setRenderPass(m_renderPasses.offscreen);
		renderPassBeginInfo.setFramebuffer(camera->frameBuffer);
		renderPassBeginInfo.renderArea.setExtent(vk::Extent2D(m_graphics->getWidth(), m_graphics->getHeight()));
		renderPassBeginInfo.renderArea.offset = { 0, 0 };
		renderPassBeginInfo.setClearValueCount(static_cast<uint32_t>(clearValues.size()));
		renderPassBeginInfo.setPClearValues(clearValues.data());

		// Inheritance info for the meshes command buffers
		vk::CommandBufferInheritanceInfo inheritanceInfo = {};
		inheritanceInfo.setRenderPass(m_renderPasses.offscreen);
		inheritanceInfo.setFramebuffer(camera->frameBuffer);

		camera->commandBuffer.buffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eSecondaryCommandBuffers);

		std::vector<vk::CommandBuffer> commandBuffers(m_meshes.size());

		m_threadPool->wait();

		// Loop over meshes
		for (size_t i = 0; i < m_meshes.size(); ++i)
		{
			commandBuffers[i] = m_meshes[i].commandBuffer.buffer;

			m_threadPool->workers[m_meshes[i].commandBuffer.index]->addJob([this, i, inheritanceInfo, camera]()
			{
				this->drawMeshToFramebuffer(m_meshes[i], inheritanceInfo, m_meshes[i].commandBuffer.index, camera);
			});
		}

		m_threadPool->wait();

		// Execute command buffers and perform lighting
		if (commandBuffers.size() > 0)
			camera->commandBuffer.buffer.executeCommands(commandBuffers);

		camera->commandBuffer.buffer.endRenderPass();
		camera->commandBuffer.buffer.end();

		vk::SubmitInfo submitInfo = {};
		submitInfo.setCommandBufferCount(1);
		submitInfo.setPCommandBuffers(&camera->commandBuffer.buffer);
		submitInfo.setSignalSemaphoreCount(1);
		submitInfo.setPSignalSemaphores(&m_semaphores.offscreen);

		// Submit draw command
		m_graphics->getGraphicsQueue().submit(1, &submitInfo, { nullptr });

		// Do lighting
		performCameraLighting(camera);
	}

	void Renderer::performCameraLighting(Handle<VirtualCamera> camera)
	{
		std::array<vk::WriteDescriptorSet, 3> sets = {};

		vk::DescriptorImageInfo position = {};
		position.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
		position.setImageView(m_mainCamera->position->getImageView());
		position.setSampler(m_mainCamera->position->getSampler());

		vk::DescriptorImageInfo normal = {};
		normal.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
		normal.setImageView(m_mainCamera->normal->getImageView());
		normal.setSampler(m_mainCamera->normal->getSampler());

		vk::DescriptorImageInfo color = {};
		color.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
		color.setImageView(m_mainCamera->color->getImageView());
		color.setSampler(m_mainCamera->color->getSampler());

		sets[0].setDstSet(m_descriptors.lightingDescriptorSet);
		sets[0].setDstBinding(1);
		sets[0].setDstArrayElement(0);
		sets[0].setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
		sets[0].setDescriptorCount(1);
		sets[0].setPImageInfo(&position);

		sets[1].setDstSet(m_descriptors.lightingDescriptorSet);
		sets[1].setDstBinding(2);
		sets[1].setDstArrayElement(0);
		sets[1].setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
		sets[1].setDescriptorCount(1);
		sets[1].setPImageInfo(&normal);

		sets[2].setDstSet(m_descriptors.lightingDescriptorSet);
		sets[2].setDstBinding(3);
		sets[2].setDstArrayElement(0);
		sets[2].setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
		sets[2].setDescriptorCount(1);
		sets[2].setPImageInfo(&color);

		// Update lighting descriptor set
		m_graphics->getLogicalDevice().updateDescriptorSets(static_cast<uint32_t>(sets.size()), sets.data(), 0, nullptr);

		vk::CommandBufferBeginInfo cmdBufInfo = {};
		cmdBufInfo.setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse);
		cmdBufInfo.setPInheritanceInfo(nullptr);

		// Begin renderpass
		camera->lightingCommandBuffer.buffer.begin(cmdBufInfo);

		vk::RenderPassBeginInfo renderPassBeginInfo = {};
		renderPassBeginInfo.setRenderPass(m_renderPasses.lighting);
		renderPassBeginInfo.setFramebuffer(camera->frameBuffer);
		renderPassBeginInfo.renderArea.setExtent(vk::Extent2D(m_graphics->getWidth(), m_graphics->getHeight()));
		renderPassBeginInfo.renderArea.offset = { 0, 0 };
		renderPassBeginInfo.setClearValueCount(0);
		renderPassBeginInfo.setPClearValues(nullptr);

		camera->lightingCommandBuffer.buffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

		// Bind descriptor sets
		camera->lightingCommandBuffer.buffer.bindDescriptorSets
		(
			vk::PipelineBindPoint::eGraphics,
			m_lightingShader.graphicsPipelineLayout,
			0,
			1,
			&m_descriptors.lightingDescriptorSet,
			0,
			nullptr
		);

		// Bind graphics pipeline
		camera->lightingCommandBuffer.buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_lightingShader.graphicsPipeline);

		// Bind vertex and index buffer
		vk::Buffer vertexBuffer = m_screenQuad->getVertexUniformBuffer().buffer;
		vk::DeviceSize offset = 0;
		camera->lightingCommandBuffer.buffer.bindVertexBuffers(0, 1, &vertexBuffer, &offset);
		camera->lightingCommandBuffer.buffer.bindIndexBuffer(m_screenQuad->getIndexUniformBuffer().buffer, 0, vk::IndexType::eUint32);

		// Draw
		camera->lightingCommandBuffer.buffer.drawIndexed(static_cast<uint32_t>(m_screenQuad->getIndexCount()), 1, 0, 0, 0);

		camera->lightingCommandBuffer.buffer.endRenderPass();
		camera->lightingCommandBuffer.buffer.end();

		vk::PipelineStageFlags flags = vk::PipelineStageFlagBits::eAllGraphics;

		vk::SubmitInfo submitInfo = {};
		submitInfo.setCommandBufferCount(1);
		submitInfo.setPCommandBuffers(&camera->lightingCommandBuffer.buffer);
		submitInfo.setWaitSemaphoreCount(1);
		submitInfo.setPWaitSemaphores(&m_semaphores.offscreen);
		submitInfo.setSignalSemaphoreCount(1);
		submitInfo.setPSignalSemaphores(&m_semaphores.offscreen);
		submitInfo.setPWaitDstStageMask(&flags);

		// Submit draw command
		m_graphics->getGraphicsQueue().submit(1, &submitInfo, { nullptr });
	}

	CommandBuffer Renderer::createCommandBuffer(vk::CommandBufferLevel level)
	{
		// Allocate command buffer
		auto commandBuffer = m_graphics->getLogicalDevice().allocateCommandBuffers
		(
			vk::CommandBufferAllocateInfo
			(
				m_commands.pools[m_commands.poolIndex],
				level,
				1
			)
		);

		size_t poolIndex = m_commands.poolIndex;
		m_commands.poolIndex = (m_commands.poolIndex + 1) % m_commands.pools.size();

		return { commandBuffer[0], poolIndex };
	}
}