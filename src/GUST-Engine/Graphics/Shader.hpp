#pragma once

/** 
 * @file Shader.hpp
 * @brief Shader header file.
 * @author Connor J. Bramham (ReeCocho)
 */

/** Includes. */
#include <string>
#include <vector>
#include "Graphics.hpp"

namespace gust
{
	/**
	 * @class Shader
	 * @brief Contains shader modules and their graphics pipelines.
	 */
	class Shader
	{
	public:

		/**
		 * @brief Default constructor.
		 */
		Shader() = default;

		/**
		 * @brief Constructor.
		 * @param Graphics context.
		 * @param Descriptor set layouts.
		 * @param Vertex shader byte code.
		 * @param Fragment shader byte code.
		 * @param Size of data sent to vertex shader.
		 * @param Size of data sent to fragment shader.
		 * @param Number of textures used by the shader.
		 * @param Should the shader perform depth testing?
		 * @param Should the shader perform lighting calculations?
		 */
		Shader
		(
			Graphics* graphics,
			const std::vector<vk::DescriptorSetLayout>& layouts,
			const vk::RenderPass& renderPass, 
			const std::vector<char>& vertexShaderByteCode, 
			const std::vector<char>& fragmentShaderByteCode,
			size_t vertexDataSize,
			size_t fragmentDataSize,
			size_t textureCount,
			bool depthTesting,
			bool lighting
		);

		/**
		 * @brief Constructor.
		 * @param Graphics context.
		 * @param Descriptor set layouts.
		 * @param Render pass.
		 * @param Path to file containing vertex shader byte code.
		 * @param Path to file containing fragment shader byte code.
		 * @param Size of data sent to vertex shader.
		 * @param Size of data sent to fragment shader.
		 * @param Number of textures used by the shader
		 * @param Should the shader perform depth testing?
		 * @param Should the shader perform lighting calculations?
		 */
		Shader
		(
			Graphics* graphics,
			const std::vector<vk::DescriptorSetLayout>& layouts,
			const vk::RenderPass& renderPass, 
			const std::string& vertexShaderPath, 
			const std::string& fragmentShaderPath,
			size_t vertexDataSize,
			size_t fragmentDataSize,
			size_t textureCount,
			bool depthTesting,
			bool lighting
		);

		/**
		 * @brief Destructor.
		 */
		~Shader();
		
		/**
		 * @brief Get descriptor set layouts.
		 * @return Descriptor set layouts.
		 */
		inline const std::vector<vk::DescriptorSetLayout>& geDescriptorSetLayouts() const
		{
			return m_descriptorSetLayouts;
		}

		/**
		 * @brief Get size of data sent to fragment shader.
		 * @return Fragment shader data size.
		 */
		inline vk::DeviceSize getFragmentDataSize() const
		{
			return m_fragmentDataSize;
		}

		/**
		 * @brief Get size of data sent to vertex shader.
		 * @return Vertex shader data size.
		 */
		inline vk::DeviceSize getVertexDataSize() const
		{
			return m_vertexDataSize;
		}

		/**
		 * @brief Get texture descriptor set layout.
		 * @return Texture descriptor set layout.
		 */
		inline const vk::DescriptorSetLayout& getTextureDescriptorSetLayout() const
		{
			return m_textureDescriptorSetLayout;
		}

		/**
		 * @brief Get graphics pipeline.
		 * @return Graphics pipeline.
		 */
		inline const vk::Pipeline& getGraphicsPipeline() const
		{
			return m_graphicsPipeline;
		}

		/**
		 * @brief Get graphics pipeline layout.
		 * @return Graphics pipeline layout.
		 */
		inline const vk::PipelineLayout& getGraphicsPipelineLayout() const
		{
			return m_graphicsPipelineLayout;
		}

		/**
		 * @brief Get texture count.
		 * @return Texture count.
		 */
		inline uint32_t getTextureCount() const
		{
			return static_cast<uint32_t>(m_textureCount);
		}

	private:

		/**
		 * @brief Initialize shader modules.
		 */
		void initShaderModules
		(
			const std::vector<char>& vertexShaderByteCode,
			const std::vector<char>& fragmentShaderByteCode
		);

		/**
		 * @brief Create descriptor set layout.
		 */
		void initDescriptorSetLayout();

		/**
		 * @brief Create graphics pipeline.
		 * @param Render pass to use for the pipeline.
		 */
		void initGraphicsPipeline(const vk::RenderPass& renderPass);



		/** Graphics context. */
		Graphics* m_graphics = nullptr;

		/** Does the shader perform depth testing? */
		const bool m_depthTesting;

		/** Does the shader perform lighting calculations? */
		const bool m_lighting;

		/** Number of textures used by the shader. */
		const size_t m_textureCount;

		/** Fragment shader module. */
		vk::ShaderModule m_fragmentShader = {};

		/** Vertex shader module. */
		vk::ShaderModule m_vertexShader = {};

		/** Size of data sent to fragment shader. */
		vk::DeviceSize m_fragmentDataSize;

		/** Size of data sent to vertex shader. */
		vk::DeviceSize m_vertexDataSize;

		/** Shader stage creation info. */
		std::array<vk::PipelineShaderStageCreateInfo, 2> m_shaderStages;

		/** Descriptor set layouts to use. */
		const std::vector<vk::DescriptorSetLayout> m_descriptorSetLayouts;

		/** Texture descriptor set layout. */
		vk::DescriptorSetLayout m_textureDescriptorSetLayout = {};

		/** Graphics pipeline layout. */
		vk::PipelineLayout m_graphicsPipelineLayout = {};

		/** Graphics pipeline. */
		vk::Pipeline m_graphicsPipeline = {};
	};
}