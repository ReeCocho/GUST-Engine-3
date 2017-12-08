#pragma once

/**
 * @file ResourceManager.hpp
 * @brief Resource manager header file.
 * @author Connor J. Bramham (ReeCocho)
 */

/** Includes. */
#include "../Utilities/Allocators.hpp"
#include "../Graphics/Mesh.hpp"
#include "../Graphics/Material.hpp"
#include "../Graphics/Shader.hpp"
#include "../Graphics/Texture.hpp"

namespace gust
{
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
		 * @param Default mesh count.
		 * @param Default material count.
		 * @param Default shader count.
		 * @param Default texture count.
		 * @note Used internally. Do not call.
		 */
		void startup
		(
			Graphics* graphics,
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
		 * @brief Destroy a mesh.
		 * @param Mesh handle.
		 */
		void destroyMesh(const Handle<Mesh>& mesh);

	private:

		/** Graphics context. */
		Graphics* m_graphics;

		/** Mesh allocator. */
		ResourceAllocator<Mesh> m_meshAllocator;

		/** Shader allocator. */
		ResourceAllocator<Shader> m_shaderAllocator;

		/** Material allocator. */
		ResourceAllocator<Material> m_materialAllocator;

		/** Texture allocator. */
		ResourceAllocator<Texture> m_textureAllocator;
	};
}