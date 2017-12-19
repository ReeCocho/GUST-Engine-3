#include "ResourceManager.hpp"
#include "../Graphics/Renderer.hpp"

namespace gust
{
	void ResourceManager::startup
	(
		Graphics* graphics,
		Renderer* renderer,
		size_t meshCount,
		size_t materialCount,
		size_t shaderCount,
		size_t textureCount
	)
	{
		m_graphics = graphics;
		m_renderer = renderer;
		m_meshAllocator = std::make_unique<ResourceAllocator<Mesh>>(meshCount, alignof(Mesh));
		m_shaderAllocator = std::make_unique<ResourceAllocator<Shader>>(shaderCount, alignof(Shader));
		m_materialAllocator = std::make_unique<ResourceAllocator<Material>>(materialCount, alignof(Material));
		m_textureAllocator = std::make_unique<ResourceAllocator<Texture>>(textureCount, alignof(Texture));
	}

	void ResourceManager::shutdown()
	{
		m_meshAllocator = {};
		m_shaderAllocator = {};
		m_materialAllocator = {};
		m_textureAllocator = {};
	}

	Handle<Mesh> ResourceManager::createMesh(const std::string& path)
	{
		// Resize the array if necessary
		if (m_meshAllocator->getResourceCount() == m_meshAllocator->getMaxResourceCount())
			m_meshAllocator->resize(m_meshAllocator->getMaxResourceCount() + 100, true);

		// Allocate mesh and call constructor
		auto mesh = Handle<Mesh>(m_meshAllocator.get(), m_meshAllocator->allocate());
		::new(mesh.get())(Mesh)(m_graphics, path);

		return mesh;
	}

	Handle<Texture> ResourceManager::createTexture(const std::string& path, vk::Filter filtering)
	{
		// Resize the array if necessary
		if (m_textureAllocator->getResourceCount() == m_textureAllocator->getMaxResourceCount())
			m_textureAllocator->resize(m_textureAllocator->getMaxResourceCount() + 100, true);

		// Allocate mesh and call constructor
		auto texture = Handle<Texture>(m_textureAllocator.get(), m_textureAllocator->allocate());
		::new(texture.get())(Texture)(m_graphics, path, filtering);

		return texture;
	}

	Handle<Texture> ResourceManager::createTexture
	(
		vk::Image image,
		vk::ImageView imageView,
		vk::Sampler sampler,
		vk::DeviceMemory memory,
		uint32_t width,
		uint32_t height
	)
	{
		// Resize the array if necessary
		if (m_textureAllocator->getResourceCount() == m_textureAllocator->getMaxResourceCount())
			m_textureAllocator->resize(m_textureAllocator->getMaxResourceCount() + 100, true);

		// Allocate texture and call constructor
		auto texture = Handle<Texture>(m_textureAllocator.get(), m_textureAllocator->allocate());
		::new(texture.get())(Texture)(m_graphics, image, imageView, sampler, memory, width, height);

		return texture;
	}

	Handle<Shader> ResourceManager::createShader
	(
		const std::string& vertexPath,
		const std::string& fragmentPath,
		size_t vertexDataSize,
		size_t fragmentDataSize,
		size_t textureCount,
		bool depthTesting,
		bool lighting
	)
	{
		// Resize the array if necessary
		if (m_shaderAllocator->getResourceCount() == m_shaderAllocator->getMaxResourceCount())
			m_shaderAllocator->resize(m_shaderAllocator->getMaxResourceCount() + 100, true);

		// Allocate shader and call constructor
		auto shader = Handle<Shader>(m_shaderAllocator.get(), m_shaderAllocator->allocate());
		::new(shader.get())(Shader)
			(
				m_graphics, 
				{m_renderer->getStandardLayout()}, 
				m_renderer->getOffscreenRenderPass(), 
				vertexPath, 
				fragmentPath,
				vertexDataSize,
				fragmentDataSize,
				textureCount,
				depthTesting,
				lighting
			);

		return shader;
	}

	Handle<Material> ResourceManager::createMaterial(Handle<Shader> shader)
	{
		// Resize the array if necessary
		if (m_materialAllocator->getResourceCount() == m_materialAllocator->getMaxResourceCount())
			m_materialAllocator->resize(m_materialAllocator->getMaxResourceCount() + 100, true);

		// Allocate material and call constructor
		auto material = Handle<Material>(m_materialAllocator.get(), m_materialAllocator->allocate());
		::new(material.get())(Material)
		(
			m_graphics,
			shader
		);

		return material;
	}

	void ResourceManager::destroyMesh(Handle<Mesh> mesh)
	{
		m_meshAllocator->deallocate(mesh.getHandle());
	}

	void ResourceManager::destroyTexture(Handle<Texture> texture)
	{
		m_textureAllocator->deallocate(texture.getHandle());
	}

	void ResourceManager::destroyShader(Handle<Shader> shader)
	{
		m_shaderAllocator->deallocate(shader.getHandle());
	}

	void ResourceManager::destroyMaterial(Handle<Material> material)
	{
		m_materialAllocator->deallocate(material.getHandle());
	}
}