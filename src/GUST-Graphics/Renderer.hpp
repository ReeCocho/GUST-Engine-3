#pragma once

/**
 * @file Renderer.hpp
 * @brief Renderer header file.
 * @author Connor J. Bramham (ReeCocho)
 */



/**
 * @def GUST_POINT_LIGHT_COUNT
 * @brief Number of point lights supported by the graphics context.
 */
#define GUST_POINT_LIGHT_COUNT 64

/**
 * @def GUST_DIRECTIONAL_LIGHT_COUNT
 * @brief Number of directional lights supported by the graphics context.
 */
#define GUST_DIRECTIONAL_LIGHT_COUNT 8

/**
 * @def GUST_SPOT_LIGHT_COUNT
 * @brief Number of spot lights supported by the graphics context.
 */
#define GUST_SPOT_LIGHT_COUNT 16

/**
 * @def GUST_LIGHTING_FRAGMENT_SHADER_PATH
 * @brief Path to the file containing the lighting rendering fragment shader.
 */
#define GUST_LIGHTING_FRAGMENT_SHADER_PATH "./Shaders/lighting-frag.spv"

/**
 * @def GUST_LIGHTING_VERTEX_SHADER_PATH
 * @brief Path to the file containing the lighting rendering vertex shader.
 */
#define GUST_LIGHTING_VERTEX_SHADER_PATH "./Shaders/lighting-vert.spv"

/**
 * @def GUST_SCREEN_FRAGMENT_SHADER_PATH
 * @brief Path to the file containing the screen rendering fragment shader.
 */
#define GUST_SCREEN_FRAGMENT_SHADER_PATH "./Shaders/screen-frag.spv"

/**
 * @def GUST_SCREEN_VERTEX_SHADER_PATH
 * @brief Path to the file containing the screen rendering vertex shader.
 */
#define GUST_SCREEN_VERTEX_SHADER_PATH "./Shaders/screen-vert.spv"

/**
 * @def GUST_SKYBOX_MESH_PATH
 * @brief Path to the file containing the cube used for a skybox.
 */
#define GUST_SKYBOX_MESH_PATH "./Meshes/Skybox.obj"

/**
 * @def GUST_SKYBOX_VERTEX_SHADER_PATH
 * @brief Path to the file containing the skybox vertex shader.
 */
#define GUST_SKYBOX_VERTEX_SHADER_PATH "./Shaders/skybox-vert.spv"

/**
 * @def GUST_SKYBOX_FRAGMENT_SHADER_PATH
 * @brief Path to the file containing the skybox fragment shader.
 */
#define GUST_SKYBOX_FRAGMENT_SHADER_PATH "./Shaders/skybox-frag.spv"

/** Includes. */
#include <queue>
#include <Allocators.hpp>
#include <Threading.hpp>
#include "Mesh.hpp"
#include "Material.hpp"
#include "Graphics.hpp"

namespace gust
{
	/**
	 * @struct MeshData
	 * @brief Information about a mesh to render.
	 */
	struct MeshData
	{
		/** Mesh to render. */
		Handle<Mesh> mesh = Handle<Mesh>::nullHandle();

		/** Material to render the mesh with. */
		Handle<Material> material = Handle<Material>::nullHandle();

		/** Descriptor sets. */
		std::vector<vk::DescriptorSet> descriptorSets = {};

		/** Command buffer to render the mesh with. */
		CommandBuffer commandBuffer = {};

		/** Vertex data uniform buffer. */
		Buffer vertexUniformBuffer = {};

		/** Fragment data uniform buffer. */
		Buffer fragmentUniformBuffer = {};

		/** Model matrix to use for the mesh. */
		glm::mat4 model = {};
	};

	/**
	 * @struct PointLightData
	 * @brief Queued point light information.
	 * @see PointLight
	 */
	struct PointLightData
	{
		/** Position of the point light. */
		glm::vec4 position = glm::vec4();

		/** Color of the point light. */
		glm::vec4 color = glm::vec4(1, 1, 1, 1);

		/** Range of the point light. */
		float range = 8.0f;

		/** Intensity of the point light. */
		float intensity = 1.0f;
	};

	/**
	 * struct DirectionalLightData
	 * @brief Queued directional light information.
	 * @see DirectionalLight
	 */
	struct DirectionalLightData
	{
		/** Direction of the directional light. */
		glm::vec4 direction = glm::vec4();

		/** Color of the directional light. */
		glm::vec4 color = glm::vec4(1, 1, 1, 1);

		/** Intensity of the directional light. */
		float intensity = 1.0f;
	};

	/**
	 * @struct SpotLightData
	 * @brief Queued spot light information.
	 * @see SpotLight
	 */
	struct SpotLightData
	{
		/** Position of the spot light. */
		glm::vec4 position = glm::vec4();

		/** Direction of the spot light. */
		glm::vec4 direction = glm::vec4();

		/** Color of the spot light. */
		glm::vec4 color = glm::vec4(1, 1, 1, 1);

		/** Cosine of the ange of the spot light. */
		float cutOff = 0;

		/** Intensity of the spot light. */
		float intensity = 0;

		/** Range of the spot light. */
		float range = 0;
	};

	/**
	 * @struct VirtualCamera
	 * @brief Camera information used by renderers.
	 */
	struct VirtualCamera
	{
		/** Framebuffer width. */
		uint32_t width = 0;

		/** Framebuffer height. */
		uint32_t height = 0;

		/** Command buffer for drawing to. */
		CommandBuffer commandBuffer = {};

		/** Command buffer for lighting. */
		CommandBuffer lightingCommandBuffer = {};

		/** Framebuffer. */
		vk::Framebuffer frameBuffer = {};

		/** Position attachment. */
		Handle<Texture> position;

		/** Normal attachment. */
		Handle<Texture> normal;

		/** Albedo attachment. */
		Handle<Texture> color;

		/** Misc attachment. */
		Handle<Texture> misc;

		/** Depth attachment. */
		Handle<Texture> depth;

		/** Projection matrix. */
		glm::mat4 projection = {};

		/** View matrix. */
		glm::mat4 view = {};

		/** Camera position. */
		glm::vec3 viewPosition = {};

		/** Clear color. */
		glm::vec3 clearColor = { 0, 0, 0 };

		/** Skybox. */
		Handle<Cubemap> skybox = Handle<Cubemap>::nullHandle();
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
		 * @param Mesh allocator.
		 * @param Texture allocator.
		 * @param Thread count.
		 */
		void startup
		(
			Graphics* graphics, 
			ResourceAllocator<Mesh>* meshAllocator, 
			ResourceAllocator<Texture>* textureAllocator, 
			size_t threadCount
		);

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
		 * @brief Get thread count.
		 * @return Thread count.
		 */
		inline size_t getThreadCount() const
		{
			return m_threadPool->workers.size();
		}

		/**
		 * @brief Get offscreen render pass.
		 * @return Offscreen render pass.
		 */
		inline const vk::RenderPass& getOffscreenRenderPass()
		{
			return m_renderPasses.offscreen;
		}

		/**
		 * @brief Get descriptor set layouts for a standard shader.
		 * @return Descriptor set layouts for a standard shader.
		 */
		inline const vk::DescriptorSetLayout& getStandardLayout()
		{
			return m_descriptors.descriptorSetLayout;
		}

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

		/**
		 * @brief Render a point light.
		 * @param Point light to render.
		 */
		inline void draw(PointLightData& pointLight)
		{
			m_pointLights.push(pointLight);
		}

		/**
		 * @brief Render a directional light.
		 * @param Directional light to render.
		 */
		inline void draw(DirectionalLightData& directionalLight)
		{
			m_directionalLights.push(directionalLight);
		}

		/**
		 * @brief Render a spot light.
		 * @param Spot light to render.
		 */
		inline void draw(SpotLightData& spotLight)
		{
			m_spotLights.push(spotLight);
		}

		/**
		 * @brief Create a camera.
		 * @return Handle to camera.
		 */
		Handle<VirtualCamera> createCamera();

		/**
		 * @brief Destroy a camera.
		 * @param Handle to camera.
		 */
		inline void destroyCamera(const Handle<VirtualCamera>& camera)
		{
			destroyCommandBuffer(camera->commandBuffer);
			destroyCommandBuffer(camera->lightingCommandBuffer);
			m_graphics->getLogicalDevice().destroyFramebuffer(camera->frameBuffer);

			camera->color->free();
			camera->depth->free();
			camera->misc->free();
			camera->normal->free();
			camera->position->free();

			m_textureAllocator->deallocate(camera->color.getHandle());
			m_textureAllocator->deallocate(camera->depth.getHandle());
			m_textureAllocator->deallocate(camera->misc.getHandle());
			m_textureAllocator->deallocate(camera->normal.getHandle());
			m_textureAllocator->deallocate(camera->position.getHandle());

			m_cameraAllocator->deallocate(camera.getHandle());
		}

		/**
		 * @brief Set the main camera.
		 * @param New main camera.
		 * @return New main camera.
		 */
		Handle<VirtualCamera> setMainCamera(const Handle<VirtualCamera>& camera);

		/**
		 * @brief Get the main camera.
		 * @return The main camera.
		 */
		inline Handle<VirtualCamera> getMainCamera() const
		{
			return m_mainCamera;
		}

		/**
		 * @brief Create a command buffer.
		 * @param Command buffer level.
		 * @return New command buffer.
		 */
		CommandBuffer createCommandBuffer(vk::CommandBufferLevel level);

		/**
		 * @brief Destroy a command buffer.
		 * @param Command buffer to destroy.
		 */
		inline void destroyCommandBuffer(CommandBuffer commandBuffer)
		{
			m_graphics->getLogicalDevice().freeCommandBuffers(m_commands.pools[commandBuffer.index], commandBuffer.buffer);
		}

		/**
		 * @brief Set ambient light color.
		 * @param New ambient light color.
		 * @return New ambient light color.
		 */
		inline glm::vec3 setAmbientColor(glm::vec3 color)
		{
			m_lightingData.ambient = glm::vec4(color, m_lightingData.ambient.w);
			return color;
		}

		/**
		 * @brief Set ambient light intensity.
		 * @param New ambient light intensity.
		 * @return New ambient light intensity.
		 */
		inline float setAmbientIntensity(float intensity)
		{
			m_lightingData.ambient.w = intensity;
			return m_lightingData.ambient.w;
		}

		/**
		 * @brief Get ambient light color.
		 * @return Ambient light color.
		 */
		inline glm::vec3 getAmbientColor() const
		{
			return { m_lightingData.ambient.x, m_lightingData.ambient.y, m_lightingData.ambient.z };
		}

		/**
		 * @brief Get ambient light intensity.
		 * @return Ambient light intensity.
		 */
		inline float getAmbientIntensity() const
		{
			return m_lightingData.ambient.w;
		}

	private:

		/**
		 * @brief Initialize command pools.
		 */
		void initCommandPools();

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
		 * @brief Initialize lighting.
		 */
		void initLighting();

		/**
		 * @brief Initialize descriptor set layouts.
		 */
		void initDescriptorSetLayouts();

		/**
		 * @brief Initialize descriptor pool.
		 */
		void initDescriptorPool();

		/**
		 * @breif Initialize descriptor set.
		 */
		void initDescriptorSets();

		/**
		 * @brief Initialize in engine shaders.
		 */
		void initShaders();

		/**
		 * @brief Initialize command buffers.
		 */
		void initCommandBuffers();

		/**
		 * @brief Create a frame buffer attachment.
		 * @param Frame buffer format.
		 * @param Frame buffer image usage.
		 * @return New frame buffer attachment.
		 */
		FrameBufferAttachment createAttachment(vk::Format format, vk::ImageUsageFlags usage);

		/**
		 * @brief Submit lighting data.
		 */
		void submitLightingData();

		/**
		 * @brief Draw mesh to a framebuffer.
		 * @param Mesh to render.
		 * @param Command buffer inheritence info.
		 * @param Thread index.
		 * @param Camera rendering the mesh.
		 */
		void drawMeshToFramebuffer
		(
			const MeshData& mesh,
			const vk::CommandBufferInheritanceInfo& inheritanceInfo,
			size_t threadIndex,
			Handle<VirtualCamera> camera
		);

		/**
		 * @brief Draw meshes to camera framebuffer.
		 * @param Camera to draw to.
		 */
		void drawToCamera(Handle<VirtualCamera> camera);

		/**
		 * @brief Performs lighting operations on the cameras framebuffer.
		 * @param Camera to draw to.
		 */
		void performCameraLighting(Handle<VirtualCamera> camera);



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

		/**
		 * @struct Descriptors
		 * @brief Information about descriptors.
		 */
		struct Descriptors
		{
			/** Descriptor set layout for a standard material. */
			vk::DescriptorSetLayout descriptorSetLayout = {};

			/** Lighting descriptor set layout for deferred rendering. */
			vk::DescriptorSetLayout lightingDescriptorSetLayout = {};

			/** Screen descriptor set layout for deferred rendering. */
			vk::DescriptorSetLayout screenDescriptorSetLayout = {};

			/** Skybox descriptor set layout for deferred rendering. */
			vk::DescriptorSetLayout skyboxDescriptorSetLayout = {};

			/** Descriptor pool. */
			vk::DescriptorPool descriptorPool = {};

			/** Lighting descriptor set. */
			vk::DescriptorSet lightingDescriptorSet = {};

			/** Screen descriptor set. */
			vk::DescriptorSet screenDescriptorSet = {};

			/** Skybox descriptor set. */
			vk::DescriptorSet skyboxDescriptorSet = {};

		} m_descriptors;

		/**
		 * @struct LightingShader
		 * @brief Data about the lighting shader.
		 */
		struct LightingShader
		{
			/** Fragment shader module. */
			vk::ShaderModule fragmentShader = {};

			/** Vertex shader module. */
			vk::ShaderModule vertexShader = {};

			/** Shader stage creation info. */
			std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStages;

			/** Texture descriptor set layout. */
			vk::DescriptorSetLayout textureDescriptorSetLayout = {};

			/** Graphics pipeline layout. */
			vk::PipelineLayout graphicsPipelineLayout = {};

			/** Graphics pipeline. */
			vk::Pipeline graphicsPipeline = {};

		} m_lightingShader;

		/**
		 * @struct ScreenShader
		 * @brief Data about the screen shader.
		 */
		struct ScreenShader
		{
			/** Fragment shader module. */
			vk::ShaderModule fragmentShader = {};

			/** Vertex shader module. */
			vk::ShaderModule vertexShader = {};

			/** Shader stage creation info. */
			std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStages;

			/** Texture descriptor set layout. */
			vk::DescriptorSetLayout textureDescriptorSetLayout = {};

			/** Graphics pipeline layout. */
			vk::PipelineLayout graphicsPipelineLayout = {};

			/** Graphics pipeline. */
			vk::Pipeline graphicsPipeline = {};

		} m_screenShader;

		/**
		 * @struct SkyboxShader
		 * @brief Data about the skybox shader.
		 */
		struct SkyboxShader
		{
			/** Fragment shader module. */
			vk::ShaderModule fragmentShader = {};

			/** Vertex shader module. */
			vk::ShaderModule vertexShader = {};

			/** Shader stage creation info. */
			std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStages;

			/** Texture descriptor set layout. */
			vk::DescriptorSetLayout textureDescriptorSetLayout = {};

			/** Graphics pipeline layout. */
			vk::PipelineLayout graphicsPipelineLayout = {};

			/** Graphics pipeline. */
			vk::Pipeline graphicsPipeline = {};

		} m_skyboxShader;

		/**
		 * @struct LightingData
		 * @brief Lighting data.
		 */
		struct LightingData
		{
			/** Point lights. */
			PointLightData pointLights[GUST_POINT_LIGHT_COUNT] = {};

			/** Point lights in use. */
			uint32_t pointLightCount = 0;

			/** Padding. */
			char padding1[12];

			/** Directional lights. */
			DirectionalLightData directionalLights[GUST_DIRECTIONAL_LIGHT_COUNT] = {};

			/** Directional lights in use. */
			uint32_t directionalLightCount = 0;

			/** Padding. */
			char padding2[12];

			/** Spot lights. */
			SpotLightData spotLights[GUST_SPOT_LIGHT_COUNT] = {};

			/** Spot lights in use. */
			uint32_t spotLightCount = 0;

			/** Padding. */
			char padding3[12];

			/** Camera view position */
			glm::vec4 viewPosition = {};

			/** Ambient color. */
			glm::vec4 ambient = { 1, 1, 1, 0.1f };

		} m_lightingData;

		/**
		 * @struct Commands
		 * @brief Command buffers and pools.
		 */
		struct Commands
		{
			/** Command pools. */
			std::vector<vk::CommandPool> pools = {};

			/** Pool index to create the next buffer on. */
			size_t poolIndex = 0;

			/** Rendering command buffers. */
			std::vector<CommandBuffer> rendering = {};

			/** Skybox command buffer. */
			CommandBuffer skybox = {};

		} m_commands;

		/** Camera allocator. */
		std::unique_ptr<ResourceAllocator<VirtualCamera>> m_cameraAllocator;

		/** Mesh allocator. */
		ResourceAllocator<Mesh>* m_meshAllocator = nullptr;

		/** Texture allocator. */
		ResourceAllocator<Texture>* m_textureAllocator = nullptr;



		/** Thread pool for rendering meshes. */
		std::unique_ptr<ThreadPool> m_threadPool = nullptr;

		/** Screen quad. */
		Handle<Mesh> m_screenQuad = Handle<Mesh>::nullHandle();

		/** Skybox. */
		Handle<Mesh> m_skybox = Handle<Mesh>::nullHandle();

		/** Main camera. */
		Handle<VirtualCamera> m_mainCamera = Handle<VirtualCamera>::nullHandle();

		/** Uniform buffer for lighting data. */
		Buffer m_lightingUniformBuffer = {};

		/** Skybox uniform buffer. */
		Buffer m_skyboxUniformBuffer = {};

		/** List of meshes to render. */
		std::vector<MeshData> m_meshes = {};

		/** Point lights to be rendered. */
		std::queue<PointLightData> m_pointLights = std::queue<PointLightData>();

		/** Directional lights to be rendered. */
		std::queue<DirectionalLightData> m_directionalLights = std::queue<DirectionalLightData>();

		/** Spot lights to be rendered. */
		std::queue<SpotLightData> m_spotLights = std::queue<SpotLightData>();
	};
}
