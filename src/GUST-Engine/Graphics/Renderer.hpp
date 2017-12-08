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
		 * @brief Initialize render pass.
		 */
		void initRenderPass();

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
		Graphics* m_graphics;

		/** Swap chain. */
		vk::SwapchainKHR m_swapchain = {};

		/** Renderpass */
		vk::RenderPass m_renderPass = {};

		/** Swap chain images. */
		SwapChainImages m_images = {};

		/** Swap chain buffers. */
		std::vector<SwapChainBuffer> m_buffers = {};

		/** Thread pool for rendering meshes. */
		std::unique_ptr<ThreadPool> m_threadPool;

		/** List of meshes to render. */
		std::vector<MeshData> m_meshes = {};
	};
}