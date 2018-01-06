#include "ResourceManager.hpp"
#include <Renderer.hpp>

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
		m_meshAllocator = std::make_unique<ResourceAllocator<Mesh>>(meshCount);
		m_shaderAllocator = std::make_unique<ResourceAllocator<Shader>>(shaderCount);
		m_materialAllocator = std::make_unique<ResourceAllocator<Material>>(materialCount);
		m_textureAllocator = std::make_unique<ResourceAllocator<Texture>>(textureCount);
	}

	void ResourceManager::shutdown()
	{
		// Free meshes
		for (size_t i = 0; i < m_meshAllocator->getMaxResourceCount(); ++i)
			if (m_meshAllocator->isAllocated(i))
				m_meshAllocator->getResourceByHandle(i)->free();

		// Free materials
		for (size_t i = 0; i < m_materialAllocator->getMaxResourceCount(); ++i)
			if (m_materialAllocator->isAllocated(i))
				m_materialAllocator->getResourceByHandle(i)->free();

		// Free shaders
		for (size_t i = 0; i < m_shaderAllocator->getMaxResourceCount(); ++i)
			if (m_shaderAllocator->isAllocated(i))
				m_shaderAllocator->getResourceByHandle(i)->free();

		// Free textures
		for (size_t i = 0; i < m_textureAllocator->getMaxResourceCount(); ++i)
			if (m_textureAllocator->isAllocated(i))
				m_textureAllocator->getResourceByHandle(i)->free();

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
		// *mesh.get() = Mesh(m_graphics, path);
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
		// *texture.get() = Texture(m_graphics, path, filtering);
		::new(texture.get())(Texture)(m_graphics, path, filtering);

		return texture;
	}

	Handle<Cubemap> ResourceManager::createCubemap
	(
		const std::string& top,
		const std::string& bottom,
		const std::string& north,
		const std::string& east,
		const std::string& south,
		const std::string& west,
		vk::Filter filter
	)
	{
		// Resize the array if necessary
		if (m_textureAllocator->getResourceCount() == m_textureAllocator->getMaxResourceCount())
			m_textureAllocator->resize(m_textureAllocator->getMaxResourceCount() + 100, true);

		// Allocate mesh and call constructor
		auto cubemap = Handle<Cubemap>(m_textureAllocator.get(), m_textureAllocator->allocate());
		// *cubemap.get() = Cubemap(m_graphics, top, bottom, north, east, south, west, filter);
		::new(cubemap.get())(Cubemap)(m_graphics, top, bottom, north, east, south, west, filter);

		return cubemap;
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
		// *texture.get() = Texture(m_graphics, image, imageView, sampler, memory, width, height);
		::new(texture.get())(Texture)(m_graphics, image, imageView, sampler, memory, width, height);

		return texture;
	}

	Handle<Cubemap> ResourceManager::createCubemap
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
		auto cubemap = Handle<Cubemap>(m_textureAllocator.get(), m_textureAllocator->allocate());
		// *cubemap.get() = Cubemap(m_graphics, image, imageView, sampler, memory, width, height);
		::new(cubemap.get())(Cubemap)(m_graphics, image, imageView, sampler, memory, width, height);

		return cubemap;
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
		/*
		*shader.get() = Shader
		(
			m_graphics,
			{ m_renderer->getStandardLayout() },
			m_renderer->getOffscreenRenderPass(),
			vertexPath,
			fragmentPath,
			vertexDataSize,
			fragmentDataSize,
			textureCount,
			depthTesting,
			lighting
		);
		*/

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
		// *material.get() = Material(m_graphics, shader);
		::new(material.get())(Material)(m_graphics, shader);

		return material;
	}

	void ResourceManager::destroyMesh(Handle<Mesh> mesh)
	{
		mesh->free();
		m_meshAllocator->deallocate(mesh.getHandle());
	}

	void ResourceManager::destroyTexture(Handle<Texture> texture)
	{
		texture->free();
		m_textureAllocator->deallocate(texture.getHandle());
	}

	void ResourceManager::destroyCubemap(Handle<Cubemap> cubemap)
	{
		cubemap->free();
		m_textureAllocator->deallocate(cubemap.getHandle());
	}

	void ResourceManager::destroyShader(Handle<Shader> shader)
	{
		shader->free();
		m_shaderAllocator->deallocate(shader.getHandle());
	}

	void ResourceManager::destroyMaterial(Handle<Material> material)
	{
		material->free();
		m_materialAllocator->deallocate(material.getHandle());
	}
}