#include <FileIO.hpp>
#include "Shader.hpp"
#include "Mesh.hpp"

namespace gust
{
	Shader::Shader
	(
		Graphics* graphics,
		const std::vector<vk::DescriptorSetLayout>& layouts,
		const vk::RenderPass& renderPass, 
		const std::vector<char>& vertexShaderByteCode,
		const std::vector<char>& fragmentShaderByteCode,
		size_t vertexDataSize,
		size_t textureCount,
		size_t fragmentDataSize,
		bool depthTesting,
		bool lighting
	) : 
		m_graphics(graphics), 
		m_descriptorSetLayouts(layouts),
		m_fragmentDataSize(static_cast<vk::DeviceSize>(fragmentDataSize)),
		m_vertexDataSize(static_cast<vk::DeviceSize>(vertexDataSize)),
		m_textureCount(textureCount),
		m_depthTesting(depthTesting),
		m_lighting(lighting)
	{
		if (m_fragmentDataSize == 0)
			m_fragmentDataSize = 1;

		if (m_vertexDataSize == 0)
			m_vertexDataSize = 1;

		initShaderModules(vertexShaderByteCode, fragmentShaderByteCode);
		initDescriptorSetLayout();
		initGraphicsPipeline(renderPass);
	}

	Shader::Shader
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
	) : 
		m_graphics(graphics),
		m_descriptorSetLayouts(layouts),
		m_fragmentDataSize(static_cast<vk::DeviceSize>(fragmentDataSize)),
		m_vertexDataSize(static_cast<vk::DeviceSize>(vertexDataSize)),
		m_textureCount(textureCount),
		m_depthTesting(depthTesting),
		m_lighting(lighting)
	{
		// Read files
		auto vertexShaderByteCode = readBinary(vertexShaderPath);
		auto fragmentShaderByteCode = readBinary(fragmentShaderPath);

		if (m_fragmentDataSize == 0)
			m_fragmentDataSize = 1;
	
		if (m_vertexDataSize == 0)
			m_vertexDataSize = 1;
	
		initShaderModules(vertexShaderByteCode, fragmentShaderByteCode);
		initDescriptorSetLayout();
		initGraphicsPipeline(renderPass);
	}

	Shader::~Shader()
	{
		auto logicalDevice = m_graphics->getLogicalDevice();

		// Destroy shader modules
		if (m_fragmentShader)
			logicalDevice.destroyShaderModule(m_fragmentShader);

		if(m_vertexShader)
			logicalDevice.destroyShaderModule(m_vertexShader);

		// Destroy graphics pipeline and layout
		if(m_graphicsPipeline)
			logicalDevice.destroyPipeline(m_graphicsPipeline);

		if(m_graphicsPipelineLayout)
			logicalDevice.destroyPipelineLayout(m_graphicsPipelineLayout);

		// Destroy descriptor set layout
		if(m_textureDescriptorSetLayout)
			logicalDevice.destroyDescriptorSetLayout(m_textureDescriptorSetLayout);
	}

	void Shader::initShaderModules
	(
		const std::vector<char>& vertexShaderByteCode,
		const std::vector<char>& fragmentShaderByteCode
	)
	{
		auto logicalDevice = m_graphics->getLogicalDevice();

		// Vertex shader module
		{
			// Align code
			std::vector<uint32_t> codeAligned(vertexShaderByteCode.size() / sizeof(uint32_t) + 1);
			memcpy(codeAligned.data(), vertexShaderByteCode.data(), vertexShaderByteCode.size());

			vk::ShaderModuleCreateInfo createInfo = {};
			createInfo.setCodeSize(vertexShaderByteCode.size());
			createInfo.setPCode(codeAligned.data());

			// Create shader module
			m_vertexShader = logicalDevice.createShaderModule(createInfo);
		}

		// Fragment shader module
		{
			// Align code
			std::vector<uint32_t> codeAligned(fragmentShaderByteCode.size() / sizeof(uint32_t) + 1);
			memcpy(codeAligned.data(), fragmentShaderByteCode.data(), fragmentShaderByteCode.size());

			vk::ShaderModuleCreateInfo createInfo = {};
			createInfo.setCodeSize(fragmentShaderByteCode.size());
			createInfo.setPCode(codeAligned.data());

			// Create shader module
			m_fragmentShader = logicalDevice.createShaderModule(createInfo);
		}

		// Vertex shader stage create info
		vk::PipelineShaderStageCreateInfo vertShaderStageInfo = {};
		vertShaderStageInfo.setStage(vk::ShaderStageFlagBits::eVertex);
		vertShaderStageInfo.setModule(m_vertexShader);
		vertShaderStageInfo.setPName("main");

		// Fragment shader stage create info
		vk::PipelineShaderStageCreateInfo fragShaderStageInfo = {};
		fragShaderStageInfo.setStage(vk::ShaderStageFlagBits::eFragment);
		fragShaderStageInfo.setModule(m_fragmentShader);
		fragShaderStageInfo.setPName("main");

		m_shaderStages = { vertShaderStageInfo, fragShaderStageInfo };
	}

	void Shader::initDescriptorSetLayout()
	{
		std::vector<vk::DescriptorSetLayoutBinding> bindings(m_textureCount);

		// Texture bindings
		for (size_t i = 0; i < m_textureCount; ++i)
		{
			bindings[i].setBinding(static_cast<uint32_t>(i));
			bindings[i].setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
			bindings[i].setDescriptorCount(1);
			bindings[i].setStageFlags(vk::ShaderStageFlagBits::eFragment);
		}

		vk::DescriptorSetLayoutCreateInfo createInfo = {};
		createInfo.setBindingCount(static_cast<uint32_t>(bindings.size()));
		createInfo.setPBindings(bindings.data());

		// Create descriptor set layout
		m_textureDescriptorSetLayout = m_graphics->getLogicalDevice().createDescriptorSetLayout(createInfo);
	}

	void Shader::initGraphicsPipeline(const vk::RenderPass& renderPass)
	{
		auto bindingDescription = Vertex::getBindingDescription();
		auto attributeDescriptions = Vertex::getAttributeDescriptions();
		
		// Vertex input
		vk::PipelineVertexInputStateCreateInfo vertexInputInfo = {};
		vertexInputInfo.setVertexBindingDescriptionCount(1);
		vertexInputInfo.setVertexAttributeDescriptionCount(static_cast<uint32_t>(attributeDescriptions.size()));
		vertexInputInfo.setPVertexBindingDescriptions(&bindingDescription);
		vertexInputInfo.setPVertexAttributeDescriptions(attributeDescriptions.data());
		
		// Input assembly
		vk::PipelineInputAssemblyStateCreateInfo inputAssembly = {};
		inputAssembly.setTopology(vk::PrimitiveTopology::eTriangleList);
		inputAssembly.setPrimitiveRestartEnable(false);
		
		// Viewport info
		vk::Viewport viewport = {};
		viewport.setX(0.0f);
		viewport.setY(0.0f);
		viewport.setWidth(static_cast<float>(m_graphics->getWidth()));
		viewport.setHeight(static_cast<float>(m_graphics->getHeight()));
		viewport.setMinDepth(0.0f);
		viewport.setMaxDepth(1.0f);
		
		// Extents
		vk::Extent2D extents = {};
		extents.setHeight(m_graphics->getHeight());
		extents.setWidth(m_graphics->getWidth());
		
		// Sicssor info
		vk::Rect2D scissor = {};
		scissor.setOffset({ 0, 0 });
		scissor.setExtent(extents);
		
		// Viewport state
		vk::PipelineViewportStateCreateInfo viewportState = {};
		viewportState.setViewportCount(1);
		viewportState.setPViewports(&viewport);
		viewportState.setScissorCount(1);
		viewportState.setPScissors(&scissor);
		
		// Rasterization
		vk::PipelineRasterizationStateCreateInfo rasterizer = {};
		rasterizer.setDepthClampEnable(false);
		rasterizer.setRasterizerDiscardEnable(false);
		rasterizer.setPolygonMode(vk::PolygonMode::eFill);
		rasterizer.setLineWidth(1.0f);
		rasterizer.setCullMode(vk::CullModeFlagBits::eBack);
		rasterizer.setFrontFace(vk::FrontFace::eClockwise);
		rasterizer.setDepthBiasEnable(false);
		rasterizer.setDepthBiasConstantFactor(0.0f);
		rasterizer.setDepthBiasClamp(0.0f);
		rasterizer.setDepthBiasSlopeFactor(0.0f);
		
		// Multisampling
		vk::PipelineMultisampleStateCreateInfo multisampling = {};
		multisampling.setSampleShadingEnable(false);
		multisampling.setRasterizationSamples(vk::SampleCountFlagBits::e1);
		multisampling.setMinSampleShading(1.0f);
		multisampling.setPSampleMask(nullptr);
		multisampling.setAlphaToCoverageEnable(false);
		multisampling.setAlphaToOneEnable(false);
		
		// Lighting stencil
		vk::StencilOpState stencil = {};
		stencil.setFailOp(vk::StencilOp::eKeep);
		stencil.setPassOp(vk::StencilOp::eReplace);
		stencil.setDepthFailOp(vk::StencilOp::eKeep);
		stencil.setCompareOp(vk::CompareOp::eAlways);
		stencil.setWriteMask(1);
		stencil.setReference(m_lighting ? 1 : 0);
		stencil.setCompareMask(1);

		// Depth testing
		vk::PipelineDepthStencilStateCreateInfo depthStencil = {};
		depthStencil.setDepthTestEnable(m_depthTesting);
		depthStencil.setDepthWriteEnable(m_depthTesting);
		depthStencil.setDepthCompareOp(vk::CompareOp::eLess);
		depthStencil.setDepthBoundsTestEnable(false);
		depthStencil.setStencilTestEnable(true);
		depthStencil.setFront(stencil);
		depthStencil.setBack(stencil);
		
		std::array<vk::PipelineColorBlendAttachmentState, 4> colorBlendAttachments = {};
		
		for (size_t i = 0; i < colorBlendAttachments.size(); ++i)
		{
			colorBlendAttachments[i].setColorWriteMask
			(
				vk::ColorComponentFlagBits::eR | 
				vk::ColorComponentFlagBits::eG | 
				vk::ColorComponentFlagBits::eB | 
				vk::ColorComponentFlagBits::eA
			);
			
			// Disable transparency for now
			colorBlendAttachments[i].setBlendEnable(false);
			colorBlendAttachments[i].setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha);
			colorBlendAttachments[i].setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha);
			colorBlendAttachments[i].setColorBlendOp(vk::BlendOp::eAdd);
			colorBlendAttachments[i].setSrcAlphaBlendFactor(vk::BlendFactor::eOne);
			colorBlendAttachments[i].setDstAlphaBlendFactor(vk::BlendFactor::eZero);
			colorBlendAttachments[i].setAlphaBlendOp(vk::BlendOp::eAdd);
		}
		
		vk::PipelineColorBlendStateCreateInfo colorBlending = {};
		colorBlending.setLogicOpEnable(false);
		colorBlending.setLogicOp(vk::LogicOp::eCopy);
		colorBlending.setAttachmentCount(static_cast<uint32_t>(colorBlendAttachments.size()));
		colorBlending.setPAttachments(colorBlendAttachments.data());
		colorBlending.setBlendConstants({ 0.0f, 0.0f, 0.0f, 0.0f });
		
		std::array<vk::DynamicState, 2> dynamicStates =
		{
			vk::DynamicState::eViewport,
			vk::DynamicState::eScissor
		};
		
		vk::PipelineDynamicStateCreateInfo dynamicState = {};
		dynamicState.setDynamicStateCount(static_cast<uint32_t>(dynamicStates.size()));
		dynamicState.setPDynamicStates(dynamicStates.data());
		
		// Layouts
		auto layouts = m_descriptorSetLayouts;
		if (m_textureCount > 0)
			layouts.push_back(m_textureDescriptorSetLayout);

		vk::PipelineLayoutCreateInfo pipelineLayoutInfo = {};	
		pipelineLayoutInfo.setSetLayoutCount(static_cast<uint32_t>(layouts.size()));
		pipelineLayoutInfo.setPSetLayouts(layouts.data());
		
		auto logicalDevice = m_graphics->getLogicalDevice();

		// Create pipeline layout
		m_graphicsPipelineLayout = logicalDevice.createPipelineLayout(pipelineLayoutInfo);
		
		vk::GraphicsPipelineCreateInfo pipelineInfo = {};
		pipelineInfo.setStageCount(static_cast<uint32_t>(m_shaderStages.size()));
		pipelineInfo.setPStages(m_shaderStages.data());
		pipelineInfo.setPVertexInputState(&vertexInputInfo);
		pipelineInfo.setPInputAssemblyState(&inputAssembly);
		pipelineInfo.setPViewportState(&viewportState);
		pipelineInfo.setPRasterizationState(&rasterizer);
		pipelineInfo.setPMultisampleState(&multisampling);
		pipelineInfo.setPColorBlendState(&colorBlending);
		pipelineInfo.setPDynamicState(&dynamicState);
		pipelineInfo.setLayout(m_graphicsPipelineLayout);
		pipelineInfo.setRenderPass(renderPass);
		pipelineInfo.setSubpass(0);
		pipelineInfo.setBasePipelineHandle({ nullptr });
		pipelineInfo.setBasePipelineIndex(-1);
		pipelineInfo.setPDepthStencilState(&depthStencil);
		
		// Create graphics pipeline
		m_graphicsPipeline = logicalDevice.createGraphicsPipeline({}, pipelineInfo);
	}
}