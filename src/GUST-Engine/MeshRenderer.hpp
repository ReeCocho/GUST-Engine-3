#pragma once

/**
 * @file MeshRenderer.hpp
 * @brief Mesh renderer header file.
 * @author Connor J. Bramham (ReeCocho)
 */

/** Includes. */
#include <Math.hpp>
#include <Scene.hpp>
#include <Renderer.hpp>

#include "Engine.hpp"
#include "Transform.hpp"

namespace gust
{
	/**
	 * @class MeshRenderer
	 * @brief Allows an entity to be rendered on screen.
	 */
	class MeshRenderer : public Component<MeshRenderer>
	{
		friend class MeshRendererSystem;

	public:

		/**
		 * @brief Default constructor.
		 */
		MeshRenderer() = default;

		/**
		 * @brief Constructor.
		 * @param Entity the component is attached to
		 * @param Component handle
		 */
		MeshRenderer(Entity entity, Handle<MeshRenderer> handle);

		/**
		 * @brief Destructor.
		 * @see Component::~Component
		 */
		~MeshRenderer();

		/**
		 * @brief Set material.
		 * @param New material.
		 * @return New material.
		 */
		inline Handle<Material> setMaterial(Handle<Material> material)
		{
			m_material = material;
			
			if (m_material != Handle<Material>::nullHandle())
			{
				std::array<vk::WriteDescriptorSet, 2> writeSets = {};
				
				// Vertex buffer
				vk::DescriptorBufferInfo gustVertexBufferInfo = {};
				gustVertexBufferInfo.setBuffer(m_material->getVertexUniformBuffer().buffer);
				gustVertexBufferInfo.setOffset(0);
				gustVertexBufferInfo.setRange(m_material->getShader()->getVertexDataSize());
				
				writeSets[0].setDstSet(m_descriptorSet);
				writeSets[0].setDstBinding(1);
				writeSets[0].setDstArrayElement(0);
				writeSets[0].setDescriptorType(vk::DescriptorType::eUniformBuffer);
				writeSets[0].setDescriptorCount(1);
				writeSets[0].setPBufferInfo(&gustVertexBufferInfo);
				writeSets[0].setPImageInfo(nullptr);
				writeSets[0].setPTexelBufferView(nullptr);
				
				// Fragment buffer
				vk::DescriptorBufferInfo gustFragmentBufferInfo = {};
				gustFragmentBufferInfo.setBuffer(m_material->getFragmentUniformBuffer().buffer);
				gustFragmentBufferInfo.setOffset(0);
				gustFragmentBufferInfo.setRange(m_material->getShader()->getFragmentDataSize());
				
				writeSets[1].setDstSet(m_descriptorSet);
				writeSets[1].setDstBinding(3);
				writeSets[1].setDstArrayElement(0);
				writeSets[1].setDescriptorType(vk::DescriptorType::eUniformBuffer);
				writeSets[1].setDescriptorCount(1);
				writeSets[1].setPBufferInfo(&gustFragmentBufferInfo);
				writeSets[1].setPImageInfo(nullptr);
				writeSets[1].setPTexelBufferView(nullptr);
				
				// Update descriptor sets
				Engine::get().graphics.getLogicalDevice().updateDescriptorSets(static_cast<uint32_t>(writeSets.size()), writeSets.data(), 0, nullptr);
			}
			
			return m_material;
		}

		/**
		 * @brief Set mesh.
		 * @param New mesh.
		 * @return New mesh.
		 */
		inline Handle<Mesh> setMesh(Handle<Mesh> mesh)
		{
			m_mesh = mesh;
			return m_mesh;
		}

		/**
		 * @brief Get material.
		 * @return Material.
		 */
		inline Handle<Material> getMaterial() const
		{
			return m_material;
		}

		/**
		 * @brief Get mesh.
		 * @return Mesh.
		 */
		inline Handle<Mesh> getMesh() const
		{
			return m_mesh;
		}

	private:

		/** Transform component. */
		Handle<Transform> m_transform = Handle<Transform>::nullHandle();

		/** Material to use when rendering. */
		Handle<Material> m_material = Handle<Material>::nullHandle();

		/** Mesh to use when rendering. */
		Handle<Mesh> m_mesh = Handle<Mesh>::nullHandle();

		/** Command buffer to use when rendering. */
		CommandBuffer m_commandBuffer = {};
		
		/** Fragment uniform buffer. */
		Buffer m_fragmentUniformBuffer = {};
		
		/** Vertex uniform buffer. */
		Buffer m_vertexUniformBuffer = {};
		
		/** Descriptor pool. */
		vk::DescriptorPool m_descriptorPool = {};
		
		/** Vertex descriptor set. */
		vk::DescriptorSet m_descriptorSet = {};
	};



	/**
	 * @class MeshRendererSystem
	 * @brief Implementation of MeshRenderers.
	 */
	class MeshRendererSystem : public System
	{
	public:

		/**
		 * @brief Constructor.
		 * @param Scene the system is in.
		 */
		MeshRendererSystem(Scene* scene);

		/**
		 * @brief Destructor.
		 */
		~MeshRendererSystem();

		/**
		 * @brief Called when a component is added to the system.
		 * @param Component to act upon.
		 */
		void onBegin() override;

		/**
		 * @brief Called once per tick, after onLateTick(), and before rendering.
		 * @param Delta time.
		 */
		void onPreRender(float deltaTime) override;

		/**
		 * @brief Called when a component is removed from the system.
		 */
		void onEnd() override;
	};
}