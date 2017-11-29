#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include "Mesh.hpp"

namespace gust
{
	Mesh::Mesh(Graphics* graphics, const std::string& path) : m_graphics(graphics)
	{
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string err;
	
		// Load OBJ
		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, path.c_str()))
			throw std::runtime_error(err);
	
		std::vector<uint32_t> indices = {};
	
		std::vector<glm::vec3> vertices = {};
		std::vector<glm::vec2> uvs = {};
		std::vector<glm::vec3> normals = {};
	
		std::vector<Vertex> uniqueVertices = {};
	
		for (const auto& shape : shapes)
			for (const auto& index : shape.mesh.indices)
			{
				Vertex vertex = {};
	
				// Get position
				vertex.position =
				{
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2]
				};
	
				// Get UV
				if (attrib.texcoords.size() > 0)
					vertex.uv =
				{
					attrib.texcoords[2 * index.texcoord_index + 0],
					1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
				};
	
				// Get normal
				if (attrib.normals.size() > 0)
					vertex.normal =
				{
					attrib.normals[3 * index.normal_index + 0],
					attrib.normals[3 * index.normal_index + 1],
					attrib.normals[3 * index.normal_index + 2]
				};
	
				// Check if duplicate vertex
				bool isUnique = true;
	
				for (size_t i = 0; i < uniqueVertices.size(); i++)
					if (uniqueVertices[i] == vertex)
					{
						indices.push_back(static_cast<uint32_t>(i));
						isUnique = false;
						break;
					}
	
				if (isUnique)
				{
					indices.push_back(static_cast<uint32_t>(vertices.size()));
					vertices.push_back(vertex.position);
					uvs.push_back(vertex.uv);
					normals.push_back(vertex.normal);
					uniqueVertices.push_back(vertex);
				}
			}
	
		m_vertices = uniqueVertices;
		m_indices = indices;
	
		initVertexBuffer();
		initIndexBuffer();
		calculateTangents();
	}

	Mesh::Mesh
	(
		Graphics* graphics,
		const std::vector<uint32_t>& indices,
		const std::vector<glm::vec3>& vertices
	) :	Mesh
	(
		graphics, 
		indices, 
		vertices, 
		std::vector<glm::vec2>(vertices.size()), 
		std::vector<glm::vec3>(vertices.size()), 
		std::vector<glm::vec3>(vertices.size())
	)
	{}

	Mesh::Mesh
	(
		Graphics* graphics,
		const std::vector<uint32_t>& indices,
		const std::vector<glm::vec3>& vertices,
		const std::vector<glm::vec2>& uvs
	) : Mesh
	(
		graphics, 
		indices, 
		vertices, 
		uvs, 
		std::vector<glm::vec3>(vertices.size()), 
		std::vector<glm::vec3>(vertices.size())
	)
	{}

	Mesh::Mesh
	(
		Graphics* graphics,
		const std::vector<uint32_t>& indices,
		const std::vector<glm::vec3>& vertices,
		const std::vector<glm::vec2>& uvs,
		const std::vector<glm::vec3>& normals
	) : Mesh
	(
		graphics, 
		indices, 
		vertices, 
		uvs, 
		normals, 
		std::vector<glm::vec3>(vertices.size())
	)
	{}

	Mesh::Mesh
	(
		Graphics* graphics,
		const std::vector<uint32_t>& indices,
		const std::vector<glm::vec3>& vertices,
		const std::vector<glm::vec2>& uvs,
		const std::vector<glm::vec3>& normals,
		const std::vector<glm::vec3>& tangents
	) :
		m_graphics(graphics),
		m_indices(indices)
	{
		m_vertices.resize(vertices.size());

		for (size_t i = 0; i < vertices.size(); i++)
		{
			m_vertices[i].position = vertices[i];
			m_vertices[i].uv = uvs[i];
			m_vertices[i].normal = normals[i];
			m_vertices[i].tangent = tangents[i];
		}

		initVertexBuffer();
		initIndexBuffer();
	}

	Mesh::~Mesh()
	{
		auto logicalDevice = m_graphics->getDeviceManager()->getLogicalDevice();

		logicalDevice.destroyBuffer(m_indexUniformBuffer.buffer);
		logicalDevice.freeMemory(m_indexUniformBuffer.memory);

		logicalDevice.destroyBuffer(m_vertexUniformBuffer.buffer);
		logicalDevice.freeMemory(m_vertexUniformBuffer.memory);
	}

	void Mesh::initVertexBuffer()
	{
		auto logicalDevice = m_graphics->getDeviceManager()->getLogicalDevice();
		const auto bufferSize = static_cast<vk::DeviceSize>(sizeof(m_vertices[0]) * m_vertices.size());

		// Create staging buffer
		Buffer stagingBuffer = m_graphics->createBuffer
		(
			bufferSize,
			vk::BufferUsageFlagBits::eTransferSrc,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
		);

		// Map memory
		void* data;
		logicalDevice.mapMemory(stagingBuffer.memory, 0, bufferSize, (vk::MemoryMapFlagBits)0, &data);
		memcpy(data, m_vertices.data(), static_cast<size_t>(bufferSize));
		logicalDevice.unmapMemory(stagingBuffer.memory);

		// Create vertex buffer
		m_vertexUniformBuffer = m_graphics->createBuffer
		(
			bufferSize,
			vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
			vk::MemoryPropertyFlagBits::eDeviceLocal
		);

		// Copy from staging buffer
		m_graphics->copyBuffer
		(
			stagingBuffer.buffer,
			m_vertexUniformBuffer.buffer,
			bufferSize
		);

		// Cleanup staging buffer
		logicalDevice.destroyBuffer(stagingBuffer.buffer);
		logicalDevice.freeMemory(stagingBuffer.memory);
	}

	void Mesh::initIndexBuffer()
	{
		auto logicalDevice = m_graphics->getDeviceManager()->getLogicalDevice();
		const auto bufferSize = static_cast<vk::DeviceSize>(sizeof(m_indices[0]) * m_indices.size());

		// Create staging buffer
		Buffer stagingBuffer = m_graphics->createBuffer
		(
			bufferSize,
			vk::BufferUsageFlagBits::eTransferSrc,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
		);

		// Load memory into buffer
		void* data;
		logicalDevice.mapMemory(stagingBuffer.memory, 0, bufferSize, (vk::MemoryMapFlagBits)0, &data);
		memcpy(data, m_indices.data(), static_cast<size_t>(bufferSize));
		logicalDevice.unmapMemory(stagingBuffer.memory);

		// Create index buffer
		m_indexUniformBuffer = m_graphics->createBuffer
		(
			bufferSize,
			vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
			vk::MemoryPropertyFlagBits::eDeviceLocal
		);

		// Copy to index buffer
		m_graphics->copyBuffer
		(
			stagingBuffer.buffer,
			m_indexUniformBuffer.buffer,
			bufferSize
		);

		// Cleanup staging buffer
		logicalDevice.destroyBuffer(stagingBuffer.buffer);
		logicalDevice.freeMemory(stagingBuffer.memory);
	}

	void Mesh::calculateTangents()
	{
		auto logicalDevice = m_graphics->getDeviceManager()->getLogicalDevice();

		logicalDevice.destroyBuffer(m_indexUniformBuffer.buffer);
		logicalDevice.freeMemory(m_indexUniformBuffer.memory);

		logicalDevice.destroyBuffer(m_vertexUniformBuffer.buffer);
		logicalDevice.freeMemory(m_vertexUniformBuffer.memory);

		for (size_t i = 0; i < m_indices.size(); i += 3)
		{
			Vertex& v0 = m_vertices[m_indices[i]];
			Vertex& v1 = m_vertices[m_indices[i + 1]];
			Vertex& v2 = m_vertices[m_indices[i + 2]];

			const auto edge1 = v1.position - v0.position;
			const auto edge2 = v2.position - v0.position;

			const auto deltaU1 = v1.uv.x - v0.uv.x;
			const auto deltaV1 = v1.uv.y - v0.uv.y;
			const auto deltaU2 = v2.uv.x - v0.uv.x;
			const auto deltaV2 = v2.uv.y - v0.uv.y;

			const auto f = 1.0f / (deltaU1 * deltaV2 - deltaU2 * deltaV1);

			glm::vec3 tangent = glm::vec3();

			tangent.x = f * (deltaV2 * edge1.x - deltaV1 * edge2.x);
			tangent.y = f * (deltaV2 * edge1.y - deltaV1 * edge2.y);
			tangent.z = f * (deltaV2 * edge1.z - deltaV1 * edge2.z);

			v0.tangent += tangent;
			v1.tangent += tangent;
			v2.tangent += tangent;
		}

		for (size_t i = 0; i < m_vertices.size(); i++)
			m_vertices[i].tangent = glm::normalize(m_vertices[i].tangent);

		initVertexBuffer();
		initIndexBuffer();
	}



	vk::VertexInputBindingDescription Vertex::getBindingDescription()
	{
		vk::VertexInputBindingDescription bindingDescription = {};
		bindingDescription.setBinding(0);
		bindingDescription.setStride(static_cast<uint32_t>(sizeof(Vertex)));
		bindingDescription.setInputRate(vk::VertexInputRate::eVertex);

		return bindingDescription;
	}

	std::array<vk::VertexInputAttributeDescription, 4> Vertex::getAttributeDescriptions()
	{
		std::array<vk::VertexInputAttributeDescription, 4> attributeDescriptions = {};

		// Position
		attributeDescriptions[0].setBinding(0);
		attributeDescriptions[0].setLocation(0);
		attributeDescriptions[0].setFormat(vk::Format::eR32G32B32A32Sfloat);
		attributeDescriptions[0].setOffset(offsetof(Vertex, position));

		// UV
		attributeDescriptions[1].setBinding(0);
		attributeDescriptions[1].setLocation(1);
		attributeDescriptions[1].setFormat(vk::Format::eR32G32Sfloat);
		attributeDescriptions[1].setOffset(offsetof(Vertex, uv));

		// Normal
		attributeDescriptions[2].setBinding(0);
		attributeDescriptions[2].setLocation(2);
		attributeDescriptions[2].setFormat(vk::Format::eR32G32B32A32Sfloat);
		attributeDescriptions[2].setOffset(offsetof(Vertex, normal));

		// Tangent
		attributeDescriptions[3].setBinding(0);
		attributeDescriptions[3].setLocation(3);
		attributeDescriptions[3].setFormat(vk::Format::eR32G32B32A32Sfloat);
		attributeDescriptions[3].setOffset(offsetof(Vertex, tangent));

		return attributeDescriptions;
	}

	bool Vertex::operator==(const Vertex& other) const
	{
		return position == other.position && normal == other.normal && uv == other.uv && tangent == other.tangent;
	}
}