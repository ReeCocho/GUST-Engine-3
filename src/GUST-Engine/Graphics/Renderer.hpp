#pragma once

/**
 * @file Renderer.hpp
 * @brief Renderer header file.
 * @author Connor J. Bramham (ReeCocho)
 */

/** Includes. */
#include "../Graphics/Graphics.hpp"
#include "../Core/ResourceManager.hpp"
#include "../Core/Threading.hpp"

namespace gust
{
	/**
	 * @struct MeshData
	 * @brief Information about a mesh to render.
	 */
	struct MeshData
	{
		/** Mesh to render. */
		Handle<Mesh> mesh;

		/** Material to render the mesh with. */
		Handle<Material> material;

		/** Command buffer to render the mesh with. */
		vk::CommandBuffer commandBuffer;
	};



	/**
	 * @class Renderer
	 * @brief Primary rendering engine.
	 */
	class Renderer
	{
	public:

		/**
		 * @brief Default constructor.
		 */
		Renderer() = default;

		/**
		 * @brief Default estructor.
		 */
		~Renderer() = default;

		/**
		 * @brief Initiailize renderer.
		 * @param Graphics context.
		 * @param Thread count.
		 */
		void startup(Graphics* graphics, size_t threadCount);

		/**
		 * @brief Shutdown renderer.
		 */
		void shutdown();

		/**
		 * @brief Render to the screen.
		 * @note Used internally. Do not call.
		 */
		void render();

		/**
		 * @brief Get swapchain.
		 * @return Swapchain.
		 */
		inline vk::SwapchainKHR& getSwapchain()
		{
			return m_swapchain.swapchain;
		}

		/**
		 * @brief Get swapchain buffer
		 * @param Buffer index.
		 * @return Buffer.
		 */
		inline const SwapChainBuffer& getSwapchainBuffer(size_t index) const
		{
			return m_swapchain.buffers[index];
		}
		
		/**
		 * @brief Get swapchain image count.
		 * @return Swapchain image count.
		 */
		inline size_t getImageCount() const
		{
			return m_swapchain.images.size();
		}

		/**
		 * @brief Render a mesh.
		 * @param Mesh to render.
		 */
		inline void draw(MeshData& mesh)
		{
			m_meshes.push_back(mesh);
		}

	private:

		/**
		 * @brief Initialize render passes.
		 */
		void initRenderPasses();

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
		 * @brief Initialize semaphores.
		 */
		void initSemaphores();

		/**
		 * @brief Initialize command buffers.
		 */
		void initCommandBuffers();



		/** Graphics context. */
		Graphics* m_graphics;

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

		/**
		 * @struct SwapchainInfo
		 * @brief Info about the swapchain.
		 */
		struct SwapchainInfo
		{
			/** Swap chain. */
			vk::SwapchainKHR swapchain = {};

			/** Swap chain images. */
			SwapChainImages images = {};

			/** Swap chain buffers. */
			std::vector<SwapChainBuffer> buffers = {};

		} m_swapchain;

		/**
		 * @struct RenderPasses
		 * @brief Render passes
		 */
		struct RenderPasses
		{
			/** Onscreen renderpass */
			vk::RenderPass onscreen = {};

			/** Offscreen renderpass. */
			vk::RenderPass offscreen = {};

			/** Renderpass to use when performing lighting calculations. */
			vk::RenderPass lighting = {};

		} m_renderPasses;

		/**
		 * @struct Semaphores
		 * @brief Semaphores used for rendering.
		 */
		struct Semaphores
		{
			/** Used to synchronize offscreen rendering and usage. */
			vk::Semaphore offscreen = {};

			/** Image avaliable semaphore. */
			vk::Semaphore imageAvailable = {};

			/** Rendering has finished and presentation can happen. */
			vk::Semaphore renderFinished = {};

		} m_semaphores;

		/** Primary command buffer. */
		vk::CommandBuffer m_primaryCommandBuffer;

		/** Thread pool for rendering meshes. */
		std::unique_ptr<ThreadPool> m_threadPool;

		/** List of meshes to render. */
		std::vector<MeshData> m_meshes = {};
	};
}