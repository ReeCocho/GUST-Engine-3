#pragma once

/**
 * @file VulkanSwapchainManager.hpp
 * @brief Vulkan swapchain manager header file
 * @author Connor J. Bramham (ReeCocho)
 */

/** Includes. */
#include "Graphics.hpp"



namespace gust
{
	/**
	 * @class VulkanSwapchainManager
	 * @brief Manages a swapchain and its images
	 * @see Graphics
	 */
	class VulkanSwapchainManager
	{
	public:

		/**
		 * @brief Default constructor.
		 */
		VulkanSwapchainManager() = default;

		/**
		 * @brief Constructor.
		 * @param Graphics context.
		 * @param Window width.
		 * @param Window height.
		 * @param Render pass.
		 * @param Queue family indices.
		 */
		VulkanSwapchainManager
		(
			Graphics* graphics,
			uint32_t width,
			uint32_t height,
			vk::RenderPass renderPass,
			QueueFamilyIndices qfi
		);

		/**
		 * @brief Destructor.
		 */
		~VulkanSwapchainManager();

		/**
		 * @brief Get swapchain.
		 * @return Swapchain.
		 */
		inline vk::SwapchainKHR& getSwapchain()
		{
			return m_swapchain;
		}

		/**
		 * @brief Get swapchain buffer
		 * @param Buffer index.
		 * @return Buffer.
		 */
		inline const SwapChainBuffer& getSwapchainBuffer(size_t index) const
		{
			return m_buffers[index];
		}
		
		/**
		 * @brief Get swapchain image count.
		 * @return Swapchain image count.
		 */
		inline size_t getImageCount() const
		{
			return m_images.size();
		}

	private:

		/**
		 * @brief Initialize swapchain.
		 */
		void initSwapchain();

		/**
		 * @brief Initialize depth resources.
		 */
		void initDepthResources();

		/**
		 * @brief Initialize swapchain buffers.
		 */
		void initSwapchainBuffers();



		/**
		 * @struct DepthTexture
		 * @brief Contains depth texture.
		 */
		struct DepthTexture
		{
			/** Depth image */
			vk::Image image = {};

			/** Depth image view. */
			vk::ImageView imageView = {};

			/** Depth image memory */
			vk::DeviceMemory memory = {};

		} m_depthTexture;

		/** Graphics context. */
		Graphics* m_graphics = nullptr;

		/** Queue family indices. */
		QueueFamilyIndices m_queueFamilyIndices = {};

		/** Swap chain. */
		vk::SwapchainKHR m_swapchain = {};

		/** Renderpass */
		vk::RenderPass m_renderPass = {};

		/** Window width. */
		uint32_t m_width = 0;

		/** Window height. */
		uint32_t m_height = 0;

		/** Swap chain images. */
		SwapChainImages m_images = {};

		/** Swap chain buffers. */
		std::vector<SwapChainBuffer> m_buffers = {};
	};
}