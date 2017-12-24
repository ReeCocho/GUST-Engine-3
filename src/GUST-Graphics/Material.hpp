#pragma once

/** 
 * @file Material.hpp
 * @brief Material header file.
 * @author Connor J. Bramham (ReeCocho)
 */

/** Includes. */
#include <Allocators.hpp>

#include "Shader.hpp"
#include "Texture.hpp"

namespace gust
{
	/**
	 * @class Material
	 * @brief Holds data to use when rendering a mesh.
	 */
	class Material
	{
	public:

		/**
		 * @brief Default constructor.
		 */
		Material() = default;

		/**
		 * @brief Constructor.
		 * @param Graphics context.
		 * @param Shader used by the material.
		 */
		Material(Graphics* graphics, Handle<Shader> shader);

		/**
		 * @brief Destructor.
		 */
		~Material();

		/**
		 * @brief Get shader.
		 * @return Shader.
		 */
		inline Handle<Shader> getShader() const
		{
			return m_shader;
		}

		/**
		 * @brief Get fragment uniform buffer.
		 * @return Fragment uniform buffer.
		 */
		inline const Buffer& getFragmentUniformBuffer() const
		{
			return m_fragmentUniformBuffer;
		}

		/**
		 * @brief Get vertex uniform buffer.
		 * @return Vertex uniform buffer.
		 */ 
		inline const Buffer& getVertexUniformBuffer() const
		{
			return m_vertexUniformBuffer;
		}

		/**
		 * @brief Set fragment uniform buffer data.
		 * @param Fragment uniform buffer data.
		 */
		template<class T>
		void setFragmentData(const T& data)
		{
			auto logicalDevice = m_graphics->getLogicalDevice();
			void* cpyData;

			// Map memory
			logicalDevice.mapMemory
			(
				m_fragmentUniformBuffer.memory,
				0,
				m_shader->getFragmentDataSize(),
				static_cast<vk::MemoryMapFlagBits>(0),
				&cpyData
			);

			// Copy
			memcpy(cpyData, &data, sizeof(T));

			// Unmap memory
			logicalDevice.unmapMemory(m_fragmentUniformBuffer.memory);
		}

		/**
		 * @brief Set vertex uniform buffer data.
		 * @param Vertex uniform buffer data.
		 */
		template<class T>
		void setVertexData(const T& data)
		{
			auto logicalDevice = m_graphics->getLogicalDevice();
			void* cpyData;

			// Map memory
			logicalDevice.mapMemory
			(
				m_vertexUniformBuffer.memory,
				0,
				m_shader->getVertexDataSize(),
				static_cast<vk::MemoryMapFlagBits>(0),
				&cpyData
			);

			// Copy
			memcpy(cpyData, &data, sizeof(T));

			// Unmap memory
			logicalDevice.unmapMemory(m_vertexUniformBuffer.memory);
		}

		/**
		 * @brief Set a texture.
		 * @param Texture.
		 * @param Which texture to set.
		 */
		void setTexture(Handle<Texture> texture, size_t index);
		
		/**
		 * @brief Get Nth texture.
		 * @param Texture index.
		 * @return Nth texture.
		 */
		inline Handle<Texture> getTexture(size_t index) const
		{
			return m_textures[index];
		}

		/**
		 * @brief Get texture descriptor set.
		 * @return Texture descriptor set.
		 */
		inline const vk::DescriptorSet& getTextureDescriptorSet() const
		{
			return m_textureDescriptorSet;
		}

	private:

		/**
		 * @brief Initialize descriptor pool.
		 */
		void initDescriptorPool();

		/**
		 * @brief Initialize descriptor sets.
		 */
		void initDescriptorSets();

		/** Graphics context. */
		Graphics* m_graphics = nullptr;

		/** Shader used by the material. */
		Handle<Shader> m_shader = Handle<Shader>::nullHandle();

		/** GUST fragment uniform buffer. */
		Buffer m_fragmentUniformBuffer = {};

		/** GUST vertex uniform buffer. */
		Buffer m_vertexUniformBuffer = {};

		/** Textures. */
		std::vector<Handle<Texture>> m_textures = {};

		/** Descriptor pool. */
		vk::DescriptorPool m_descriptorPool = {};

		/** Texture descriptor set. */
		vk::DescriptorSet m_textureDescriptorSet = {};
	};
}