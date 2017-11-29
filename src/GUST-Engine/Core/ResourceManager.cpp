#include "ResourceManager.hpp"

namespace gust
{
	void ResourceManager::startup
	(
		Graphics* graphics,
		size_t meshCount,
		size_t materialCount,
		size_t shaderCount,
		size_t textureCount
	)
	{
		m_graphics = graphics;
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
		Handle<Mesh> mesh = Handle<Mesh>(&m_meshAllocator, m_meshAllocator.allocate());
		::new(mesh.get())(Mesh)(m_graphics, path);

		return mesh;
	}

	void ResourceManager::destroyMesh(const Handle<Mesh>& mesh)
	{
		m_meshAllocator.deallocate(mesh.getHandle());
	}
}