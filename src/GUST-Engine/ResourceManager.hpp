#pragma once

/**
 * @file ResourceManager.hpp
 * @brief Resource manager header file.
 * @author Connor J. Bramham (ReeCocho)
 */

/** Includes. */
#include "vulkan\vulkan.hpp"
#include "Allocators.hpp"
#include "Mesh.hpp"
#include "Material.hpp"
#include "Shader.hpp"
#include "Texture.hpp"

namespace gust
{
	class Graphics;

	class Renderer;

	/**
	 * @class ResourceManager
	 * @brief Manages resources.
	 */
	class ResourceManager
	{
	public:

		/**
		 * @brief Default constructor.
		 */
		ResourceManager() = default;

		/**
		 * @brief Default destructor.
		 */
		~ResourceManager() = default;

		/**
		 * @brief Startup resource manager.
		 * @param Graphics context.
		 * @param Rendering engine.
		 * @param Default mesh count.
		 * @param Default material count.
		 * @param Default shader count.
		 * @param Default texture count.
		 * @note Used internally. Do not call.
		 */
		void startup
		(
			Graphics* graphics,
			Renderer* renderer,
			size_t meshCount,
			size_t materialCount,
			size_t shaderCount,
			size_t textureCount
		);

		/**
		 * @brief Shutdown resource manager.
		 * @note Used internally. Do not call.
		 */
		void shutdown();

		/**
		 * @brief Create a mesh.
		 * @param Path to an OBJ file containing the mesh.
		 * @return Mesh handle.
		 */
		Handle<Mesh> createMesh(const std::string& path);

		/**
		 * @brief Create a texture.
		 * @param Path to a file containing the texture.
		 * @param Texture filtering.
		 * @return Texture handle.
		 */
		Handle<Texture> createTexture(const std::string& path, vk::Filter filtering);

		/**
		 * @brief Create a cubemap.
		 * @param Path to file containing top image.
		 * @param Path to file containing bottom image.
		 * @param Path to file containing north image.
		 * @param Path to file containing east image.
		 * @param Path to file containing south image.
		 * @param Path to file containing west image.
		 * @param Texture filtering.
		 * @return Texture handle.
		 */
		Handle<Cubemap> createCubemap
		(
			const std::string& top,
			const std::string& bottom,
			const std::string& north,
			const std::string& east,
			const std::string& south,
			const std::string& west,
			vk::Filter filter
		);

		/**
		 * @brief Create a texture.
		 * @param Image.
		 * @param Image view.
		 * @param Sampler.
		 * @param Image memory.
		 * @param Width.
		 * @param Height.
		 * @return Texture handle.
		 */
		Handle<Texture> createTexture
		(
			vk::Image image, 
			vk::ImageView imageView, 
			vk::Sampler sampler, 
			vk::DeviceMemory memory, 
			uint32_t width, 
			uint32_t height
		);

		/**
		 * @brief Create a cubemap.
		 * @param Image.
		 * @param Image view.
		 * @param Sampler.
		 * @param Image memory.
		 * @param Width.
		 * @param Height.
		 * @return Texture handle.
		 */
		Handle<Cubemap> createCubemap
		(
			vk::Image image, 
			vk::ImageView imageView, 
			vk::Sampler sampler, 
			vk::DeviceMemory memory, 
			uint32_t width, 
			uint32_t height
		);
		
		/**
		 * @brief Create a shader.
		 * @param Path to vertex shader.
		 * @param Path to fragment shader.
		 * @param Size of data sent to vertex shader.
		 * @param Size of data sent to fragment shader.
		 * @param Number of textures used by the shader.
		 * @param Should the shader perform depth testing?
		 * @param Should the shader perform lighting calculations?
		 * @return Shader handle.
		 */
		Handle<Shader> createShader
		(
			const std::string& vertexPath,
			const std::string& fragmentPath,
			size_t vertexDataSize,
			size_t fragmentDataSize,
			size_t textureCount,
			bool depthTesting,
			bool lighting
		);

		/**
		 * @brief Create a material.
		 * @param Shader used by the material.
		 * @return Material handle.
		 */
		Handle<Material> createMaterial(Handle<Shader> shader);

		/**
		 * @brief Destroy a mesh.
		 * @param Mesh handle.
		 */
		void destroyMesh(Handle<Mesh> mesh);

		/**
		 * @brief Destroy a texture.
		 * @param Texture handle.
		 */
		void destroyTexture(Handle<Texture> texture);

		/**
		 * @brief Destroy a cubemap.
		 * @param Cubemap handle.
		 */
		void destroyCubemap(Handle<Cubemap> cubemap);

		/**
		 * @brief Destroy a shader.
		 * @param Shader handle.
		 */
		void destroyShader(Handle<Shader> shader);

		/**
		 * @brief Destroy a material.
		 * @param Material handle.
		 */
		void destroyMaterial(Handle<Material> material);

	private:

		/** Graphics context. */
		Graphics* m_graphics;

		/** Rendering engine. */
		Renderer* m_renderer;

		/** Mesh allocator. */
		std::unique_ptr<ResourceAllocator<Mesh>> m_meshAllocator;

		/** Shader allocator. */
		std::unique_ptr<ResourceAllocator<Shader>> m_shaderAllocator;

		/** Material allocator. */
		std::unique_ptr<ResourceAllocator<Material>> m_materialAllocator;

		/** Texture allocator. */
		std::unique_ptr<ResourceAllocator<Texture>> m_textureAllocator;
	};
}