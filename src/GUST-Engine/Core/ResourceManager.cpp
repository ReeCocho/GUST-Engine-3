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
		m_meshAllocator = ResourceAllocator<Mesh>(meshCount, 4);
		m_shaderAllocator = ResourceAllocator<Shader>(shaderCount, 4);
		m_materialAllocator = ResourceAllocator<Material>(materialCount, 4);
		m_textureAllocator = ResourceAllocator<Texture>(textureCount, 4);
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
		if (m_meshAllocator.getResourceCount() == m_meshAllocator.getMaxResourceCount())
			m_meshAllocator.resize(m_meshAllocator.getMaxResourceCount() + 100, true);

		// Allocate mesh and call constructor
		auto mesh = Handle<Mesh>(&m_meshAllocator, m_meshAllocator.allocate());
		::new(mesh.get())(Mesh)(m_graphics, path);

		return mesh;
	}

	Handle<Texture> ResourceManager::createTexture(const std::string& path, vk::Filter filtering)
	{
		// Resize the array if necessary
		if (m_textureAllocator.getResourceCount() == m_textureAllocator.getMaxResourceCount())
			m_textureAllocator.resize(m_textureAllocator.getMaxResourceCount() + 100, true);

		// Allocate mesh and call constructor
		auto texture = Handle<Texture>(&m_textureAllocator, m_textureAllocator.allocate());
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
		if (m_textureAllocator.getResourceCount() == m_textureAllocator.getMaxResourceCount())
			m_textureAllocator.resize(m_textureAllocator.getMaxResourceCount() + 100, true);

		// Allocate texture and call constructor
		auto texture = Handle<Texture>(&m_textureAllocator, m_textureAllocator.allocate());
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
		bool depthTesting
	)
	{
		// Resize the array if necessary
		if (m_shaderAllocator.getResourceCount() == m_shaderAllocator.getMaxResourceCount())
			m_shaderAllocator.resize(m_shaderAllocator.getMaxResourceCount() + 100, true);

		// Allocate shader and call constructor
		auto shader = Handle<Shader>(&m_shaderAllocator, m_shaderAllocator.allocate());
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
				depthTesting
			);

		return shader;
	}

	void ResourceManager::destroyMesh(const Handle<Mesh>& mesh)
	{
		m_meshAllocator.deallocate(mesh.getHandle());
	}

	void ResourceManager::destroyTexture(const Handle<Texture>& texture)
	{
		m_textureAllocator.deallocate(texture.getHandle());
	}

	void ResourceManager::destroyShader(const Handle<Shader>& shader)
	{
		m_shaderAllocator.deallocate(shader.getHandle());
	}
}