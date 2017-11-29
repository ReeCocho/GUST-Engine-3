#include "Material.hpp"

namespace gust
{
	Material::Material(Graphics* graphics, Shader* shader, const vk::DescriptorSetLayout& layout) : 
		m_graphics(graphics), 
		m_shader(shader)
	{
		// Create fragment uniform buffer
		m_fragmentUniformBuffer = m_graphics->createBuffer
		(
			m_shader->getFragmentDataSize(),
			vk::BufferUsageFlagBits::eUniformBuffer,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
		);

		// Create vertex uniform buffer
		m_vertexUniformBuffer = m_graphics->createBuffer
		(
			m_shader->getVertexDataSize(),
			vk::BufferUsageFlagBits::eUniformBuffer,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
		);

		m_textures.resize(shader->getTextureCount());
		initDescriptorPool();
		initDescriptorSets(layout);
	}

	Material::~Material()
	{
		auto logicalDevice = m_graphics->getDeviceManager()->getLogicalDevice();

		// Destroy pool
		logicalDevice.destroyDescriptorPool(m_descriptorPool);

		// Cleanup fragment uniform buffer
		logicalDevice.destroyBuffer(m_fragmentUniformBuffer.buffer);
		logicalDevice.freeMemory(m_fragmentUniformBuffer.memory);

		// Cleanup vertex uniform buffer
		logicalDevice.destroyBuffer(m_vertexUniformBuffer.buffer);
		logicalDevice.freeMemory(m_vertexUniformBuffer.memory);
	}

	void Material::setTexture(Texture* texture, size_t index)
	{
		m_textures[index] = texture;
	
		for (size_t i = 0; i < m_textures.size(); i++)
			if (m_textures[i] == nullptr)
				return;

		std::vector<vk::WriteDescriptorSet> writeSets(m_shader->getTextureCount());
		std::vector<vk::DescriptorImageInfo> descriptorImageInfo(m_shader->getTextureCount());

		// Textures
		for (size_t i = 0; i < m_shader->getTextureCount(); i++)
		{
			Texture* texture = m_textures[i];

			descriptorImageInfo[i].setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
			descriptorImageInfo[i].setImageView(texture->getImageView());
			descriptorImageInfo[i].setSampler(texture->getSampler());

			writeSets[i].setDstSet(m_textureDescriptorSet);
			writeSets[i].setDstBinding(static_cast<uint32_t>(i));
			writeSets[i].setDstArrayElement(0);
			writeSets[i].setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
			writeSets[i].setDescriptorCount(1);
			writeSets[i].setPImageInfo(&descriptorImageInfo[i]);
		}

		// Update descriptor sets
		m_graphics->getDeviceManager()->getLogicalDevice().updateDescriptorSets(static_cast<uint32_t>(writeSets.size()), writeSets.data(), 0, nullptr);
	}

	void Material::initDescriptorPool()
	{
		std::vector<vk::DescriptorPoolSize> poolSizes(1 + static_cast<size_t>(m_shader->getTextureCount() > 0));

		// GUST data
		poolSizes[0].setType(vk::DescriptorType::eUniformBuffer);
		poolSizes[0].setDescriptorCount(2);

		// Texture data
		if (poolSizes.size() > 1)
		{
			poolSizes[1].setType(vk::DescriptorType::eCombinedImageSampler);
			poolSizes[1].setDescriptorCount(m_shader->getTextureCount());
		}

		vk::DescriptorPoolCreateInfo poolInfo = {};
		poolInfo.setPoolSizeCount(static_cast<uint32_t>(poolSizes.size()));
		poolInfo.setPPoolSizes(poolSizes.data());
		poolInfo.setMaxSets(static_cast<uint32_t>(poolSizes.size()));

		// Create descriptor pool
		m_descriptorPool = m_graphics->getDeviceManager()->getLogicalDevice().createDescriptorPool(poolInfo);
	}

	void Material::initDescriptorSets(const vk::DescriptorSetLayout& layout)
	{
		auto logicalDevice = m_graphics->getDeviceManager()->getLogicalDevice();

		// Texture descriptor set
		if (m_shader->getTextureCount() > 0)
		{
			auto textureDescSet = m_shader->getTextureDescriptorSetLayout();
			vk::DescriptorSetAllocateInfo allocInfo = {};
			allocInfo.setDescriptorPool(m_descriptorPool);
			allocInfo.setDescriptorSetCount(1);
			allocInfo.setPSetLayouts(&textureDescSet);
			m_textureDescriptorSet = logicalDevice.allocateDescriptorSets(allocInfo)[0];
		}

		// Data descriptor set
		vk::DescriptorSetAllocateInfo allocInfo = {};
		allocInfo.setDescriptorPool(m_descriptorPool);
		allocInfo.setDescriptorSetCount(1);
		allocInfo.setPSetLayouts(&layout);
		m_dataDescriptorSet = logicalDevice.allocateDescriptorSets(allocInfo)[0];
		
		std::array<vk::WriteDescriptorSet, 2> writeSets = {};

		// Vertex buffer
		vk::DescriptorBufferInfo vertexBufferInfo = {};
		vertexBufferInfo.setBuffer(m_vertexUniformBuffer.buffer);
		vertexBufferInfo.setOffset(0);
		vertexBufferInfo.setRange(m_shader->getVertexDataSize());

		writeSets[0].setDstSet(m_dataDescriptorSet);
		writeSets[0].setDstBinding(0);
		writeSets[0].setDstArrayElement(0);
		writeSets[0].setDescriptorType(vk::DescriptorType::eUniformBuffer);
		writeSets[0].setDescriptorCount(1);
		writeSets[0].setPBufferInfo(&vertexBufferInfo);
		writeSets[0].setPImageInfo(nullptr);
		writeSets[0].setPTexelBufferView(nullptr);

		// Fragment buffer
		vk::DescriptorBufferInfo fragmentBufferInfo = {};
		fragmentBufferInfo.setBuffer(m_fragmentUniformBuffer.buffer);
		fragmentBufferInfo.setOffset(0);
		fragmentBufferInfo.setRange(m_shader->getFragmentDataSize());

		writeSets[1].setDstSet(m_dataDescriptorSet);
		writeSets[1].setDstBinding(1);
		writeSets[1].setDstArrayElement(0);
		writeSets[1].setDescriptorType(vk::DescriptorType::eUniformBuffer);
		writeSets[1].setDescriptorCount(1);
		writeSets[1].setPBufferInfo(&fragmentBufferInfo);
		writeSets[1].setPImageInfo(nullptr);
		writeSets[1].setPTexelBufferView(nullptr);

		// Update descriptor sets
		logicalDevice.updateDescriptorSets(static_cast<uint32_t>(writeSets.size()), writeSets.data(), 0, nullptr);
	}
}