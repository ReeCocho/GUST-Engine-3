#include "MeshRenderer.hpp"

namespace gust
{
	MeshRenderer::MeshRenderer(Entity entity, Handle<MeshRenderer> handle) : Component<MeshRenderer>(entity, handle)
	{

	}

	MeshRenderer::~MeshRenderer()
	{

	}



	MeshRendererSystem::MeshRendererSystem(Scene* scene) : System(scene)
	{
		initialize<MeshRenderer>();
	}

	MeshRendererSystem::~MeshRendererSystem()
	{

	}

	void MeshRendererSystem::onBegin()
	{
		auto meshRenderer = getComponent<MeshRenderer>();
		Graphics* graphics = &Engine::get().graphics;
		Renderer* renderer = &Engine::get().renderer;
		
		// Get transform
		meshRenderer->m_transform = meshRenderer->getEntity().getComponent<Transform>();
		
		// Create command buffers
		meshRenderer->m_commandBuffer = Engine::get().renderer.createCommandBuffer(vk::CommandBufferLevel::eSecondary);
		
		// Fragment uniform buffer
		meshRenderer->m_fragmentUniformBuffer = graphics->createBuffer
		(
			sizeof(FragmentShaderData),
			vk::BufferUsageFlagBits::eUniformBuffer,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
		);
		
		// Vertex uniform buffer
		meshRenderer->m_vertexUniformBuffer = graphics->createBuffer
		(
			sizeof(VertexShaderData),
			vk::BufferUsageFlagBits::eUniformBuffer,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
		);
		
		vk::DescriptorPoolSize poolSize = {};
		poolSize.setType(vk::DescriptorType::eUniformBuffer);
		poolSize.setDescriptorCount(4);
		
		vk::DescriptorPoolCreateInfo poolInfo = {};
		poolInfo.setPoolSizeCount(1);
		poolInfo.setPPoolSizes(&poolSize);
		poolInfo.setMaxSets(1);
		
		// Create descriptor pool
		meshRenderer->m_descriptorPool = graphics->getLogicalDevice().createDescriptorPool(poolInfo);
		
		auto& descSet = renderer->getStandardLayout();
		
		vk::DescriptorSetAllocateInfo allocInfo = {};
		allocInfo.setDescriptorPool(meshRenderer->m_descriptorPool);
		allocInfo.setDescriptorSetCount(1);
		allocInfo.setPSetLayouts(&descSet);
		
		// Allocate descriptor sets
		meshRenderer->m_descriptorSet = graphics->getLogicalDevice().allocateDescriptorSets(allocInfo)[0];

		std::array<vk::WriteDescriptorSet, 2> writeSets = {};
		
		// GUST vertex buffer
		vk::DescriptorBufferInfo gustVertexBufferInfo = {};
		gustVertexBufferInfo.setBuffer(meshRenderer->m_vertexUniformBuffer.buffer);
		gustVertexBufferInfo.setOffset(0);
		gustVertexBufferInfo.setRange(static_cast<vk::DeviceSize>(sizeof(VertexShaderData)));
		
		writeSets[0].setDstSet(meshRenderer->m_descriptorSet);
		writeSets[0].setDstBinding(0);
		writeSets[0].setDstArrayElement(0);
		writeSets[0].setDescriptorType(vk::DescriptorType::eUniformBuffer);
		writeSets[0].setDescriptorCount(1);
		writeSets[0].setPBufferInfo(&gustVertexBufferInfo);
		writeSets[0].setPImageInfo(nullptr);
		writeSets[0].setPTexelBufferView(nullptr);
		
		// GUST fragment buffer
		vk::DescriptorBufferInfo gustFragmentBufferInfo = {};
		gustFragmentBufferInfo.setBuffer(meshRenderer->m_fragmentUniformBuffer.buffer);
		gustFragmentBufferInfo.setOffset(0);
		gustFragmentBufferInfo.setRange(static_cast<vk::DeviceSize>(sizeof(FragmentShaderData)));
		
		writeSets[1].setDstSet(meshRenderer->m_descriptorSet);
		writeSets[1].setDstBinding(2);
		writeSets[1].setDstArrayElement(0);
		writeSets[1].setDescriptorType(vk::DescriptorType::eUniformBuffer);
		writeSets[1].setDescriptorCount(1);
		writeSets[1].setPBufferInfo(&gustFragmentBufferInfo);
		writeSets[1].setPImageInfo(nullptr);
		writeSets[1].setPTexelBufferView(nullptr);
		
		// Update descriptor sets
		graphics->getLogicalDevice().updateDescriptorSets(static_cast<uint32_t>(writeSets.size()), writeSets.data(), 0, nullptr);
	}

	void MeshRendererSystem::onPreRender(float deltaTime)
	{
		auto meshRenderer = getComponent<MeshRenderer>();
		
		if (meshRenderer->m_material != Handle<Material>::nullHandle() && meshRenderer->m_mesh != Handle<Mesh>::nullHandle())
		{
			MeshData data = {};
			data.commandBuffer = meshRenderer->m_commandBuffer;
			data.material = meshRenderer->m_material;
			data.mesh = meshRenderer->m_mesh;
			data.model = meshRenderer->m_transform->getModelMatrix();
			data.fragmentUniformBuffer = meshRenderer->m_fragmentUniformBuffer;
			data.vertexUniformBuffer = meshRenderer->m_vertexUniformBuffer;
		
			if (meshRenderer->m_material->getShader()->getTextureCount() > 0)
			{
				data.descriptorSets.resize(2);
				data.descriptorSets[0] = meshRenderer->m_descriptorSet;
				data.descriptorSets[1] = meshRenderer->m_material->getTextureDescriptorSet();
			}
			else
			{
				data.descriptorSets.resize(1);
				data.descriptorSets[0] = meshRenderer->m_descriptorSet;
			}
		
			Engine::get().renderer.draw(data);
		}
	}

	void MeshRendererSystem::onEnd()
	{
		auto meshRenderer = getComponent<MeshRenderer>();
		auto& graphics = Engine::get().graphics;
		auto& logicalDevice = graphics.getLogicalDevice();
		
		// Destroy command buffer
		Engine::get().renderer.destroyCommandBuffer(meshRenderer->m_commandBuffer);
		
		// Destroy pool
		logicalDevice.destroyDescriptorPool(meshRenderer->m_descriptorPool);
		
		// Cleanup fragment uniform buffer
		logicalDevice.destroyBuffer(meshRenderer->m_fragmentUniformBuffer.buffer);
		logicalDevice.freeMemory(meshRenderer->m_fragmentUniformBuffer.memory);
		
		// Cleanup vertex uniform buffer
		logicalDevice.destroyBuffer(meshRenderer->m_vertexUniformBuffer.buffer);
		logicalDevice.freeMemory(meshRenderer->m_vertexUniformBuffer.memory);
	}
} 