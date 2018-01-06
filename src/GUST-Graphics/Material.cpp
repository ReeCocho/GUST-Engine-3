#include "Material.hpp"

namespace gust
{
	Material::Material(Graphics* graphics, Handle<Shader> shader) : 
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

		// Set texture array
		m_textures.resize(shader->getTextureCount());
		for (size_t i = 0; i < m_textures.size(); i++)
			m_textures[i] = Handle<Texture>::nullHandle();

		initDescriptorPool();
		initDescriptorSets();
	}

	Material::~Material()
	{

	}

	void Material::free()
	{
		if (m_graphics)
		{
			auto logicalDevice = m_graphics->getLogicalDevice();

			// Destroy pool
			if(m_descriptorPool)
				logicalDevice.destroyDescriptorPool(m_descriptorPool);

			// Cleanup fragment uniform buffer
			if(m_fragmentUniformBuffer.buffer)
				logicalDevice.destroyBuffer(m_fragmentUniformBuffer.buffer);

			if(m_fragmentUniformBuffer.memory)
				logicalDevice.freeMemory(m_fragmentUniformBuffer.memory);

			// Cleanup vertex uniform buffer
			if(m_vertexUniformBuffer.buffer)
				logicalDevice.destroyBuffer(m_vertexUniformBuffer.buffer);

			if(m_vertexUniformBuffer.memory)
				logicalDevice.freeMemory(m_vertexUniformBuffer.memory);
		}
	}

	void Material::setTexture(Handle<Texture> texture, size_t index)
	{
		m_textures[index] = texture;
	
		for (size_t i = 0; i < m_textures.size(); ++i)
			if (m_textures[i] == Handle<Texture>::nullHandle())
				return;

		std::vector<vk::WriteDescriptorSet> writeSets(m_shader->getTextureCount());
		std::vector<vk::DescriptorImageInfo> descriptorImageInfo(m_shader->getTextureCount());

		// Textures
		for (size_t i = 0; i < m_shader->getTextureCount(); ++i)
		{
			Handle<Texture> texture = m_textures[i];

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
		m_graphics->getLogicalDevice().updateDescriptorSets(static_cast<uint32_t>(writeSets.size()), writeSets.data(), 0, nullptr);
	}

	void Material::initDescriptorPool()
	{
		if (m_shader->getTextureCount() > 0)
		{
			vk::DescriptorPoolSize poolSize = {};

			// Texture data
			poolSize.setType(vk::DescriptorType::eCombinedImageSampler);
			poolSize.setDescriptorCount(m_shader->getTextureCount());

			vk::DescriptorPoolCreateInfo poolInfo = {};
			poolInfo.setPoolSizeCount(1);
			poolInfo.setPPoolSizes(&poolSize);
			poolInfo.setMaxSets(1);

			// Create descriptor pool
			m_descriptorPool = m_graphics->getLogicalDevice().createDescriptorPool(poolInfo);
		}
	}

	void Material::initDescriptorSets()
	{
		// Texture descriptor set
		if (m_shader->getTextureCount() > 0)
		{
			auto logicalDevice = m_graphics->getLogicalDevice();

			auto textureDescSet = m_shader->getTextureDescriptorSetLayout();
			vk::DescriptorSetAllocateInfo allocInfo = {};
			allocInfo.setDescriptorPool(m_descriptorPool);
			allocInfo.setDescriptorSetCount(1);
			allocInfo.setPSetLayouts(&textureDescSet);
			m_textureDescriptorSet = logicalDevice.allocateDescriptorSets(allocInfo)[0];
		}
	}
}