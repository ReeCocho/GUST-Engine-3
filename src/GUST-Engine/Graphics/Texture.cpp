// #define STB_IMAGE_IMPLEMENTATION
// #include <stb_image.h>

#include "Texture.hpp"

namespace gust
{
	Texture::Texture(Graphics* graphics, vk::Image image, vk::ImageView imageView, vk::Sampler sampler, vk::DeviceMemory memory, uint32_t width, uint32_t height) :
		m_graphics(graphics),
		m_image(image),
		m_imageView(imageView),
		m_sampler(sampler),
		m_imageMemory(memory),
		m_width(width),
		m_height(height)
	{
		
	}

	// Texture::Texture(Graphics* graphics, const std::string& path, vk::Filter filter) : m_graphics(graphics), m_filtering(filter)
	// {
	// 	// Load image from file
	// 	int texWidth = 0, texHeight = 0, texChannels = 0;
	// 	auto pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	// 
	// 	m_width = static_cast<uint32_t>(texWidth);
	// 	m_height = static_cast<uint32_t>(texHeight);
	// 
	// 	if (!pixels)
	// 		throw std::runtime_error("VULKAN: Failed to load texture image");
	// 
	// 	auto imageSize = static_cast<vk::DeviceSize>(m_width * m_height * 4);
	// 
	// 	// Create staging buffer
	// 	Buffer stagingBuffer = m_graphics->createBuffer
	// 	(
	// 		imageSize,
	// 		vk::BufferUsageFlagBits::eTransferSrc,
	// 		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
	// 	);
	// 
	// 	// Map image to memory
	// 	void* data;
	// 	m_graphics->getDeviceManager()->getLogicalDevice().mapMemory(stagingBuffer.memory, 0, imageSize, (vk::MemoryMapFlagBits)0, &data);
	// 	memcpy(data, pixels, static_cast<size_t>(imageSize));
	// 	m_graphics->getDeviceManager()->getLogicalDevice().unmapMemory(stagingBuffer.memory);
	// 
	// 	// Free pixel data
	// 	stbi_image_free(pixels);
	// 
	// 	// Create image
	// 	m_graphics->createImage
	// 	(
	// 		m_width,
	// 		m_height,
	// 		vk::Format::eR8G8B8A8Unorm,
	// 		vk::ImageTiling::eOptimal,
	// 		vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
	// 		vk::MemoryPropertyFlagBits::eDeviceLocal,
	// 		m_image,
	// 		m_imageMemory
	// 	);
	// 
	// 	// Prepare texture for shader access
	// 	m_graphics->transitionImageLayout(m_image, vk::Format::eR8G8B8A8Unorm, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
	// 	m_graphics->copyBufferToImage(stagingBuffer.buffer, m_image, static_cast<uint32_t>(m_width), static_cast<uint32_t>(m_height));
	// 	m_graphics->transitionImageLayout(m_image, vk::Format::eR8G8B8A8Unorm, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
	// 
	// 	// Cleanup staging buffer
	// 	m_graphics->getDeviceManager()->getLogicalDevice().destroyBuffer(stagingBuffer.buffer);
	// 	m_graphics->getDeviceManager()->getLogicalDevice().freeMemory(stagingBuffer.memory);
	// 
	// 	// Create texture image view
	// 	m_imageView = m_graphics->createImageView(m_image, vk::Format::eR8G8B8A8Unorm, vk::ImageAspectFlagBits::eColor);
	// 
	// 	// Sampler creation info
	// 	vk::SamplerCreateInfo samplerInfo = {};
	// 	samplerInfo.setMagFilter(filter);
	// 	samplerInfo.setMinFilter(filter);
	// 	samplerInfo.setAddressModeU(vk::SamplerAddressMode::eRepeat);
	// 	samplerInfo.setAddressModeV(vk::SamplerAddressMode::eRepeat);
	// 	samplerInfo.setAddressModeW(vk::SamplerAddressMode::eRepeat);
	// 	samplerInfo.setAnisotropyEnable(true);
	// 	samplerInfo.setMaxAnisotropy(1);
	// 	samplerInfo.setBorderColor(vk::BorderColor::eFloatOpaqueBlack);
	// 	samplerInfo.setUnnormalizedCoordinates(false);
	// 	samplerInfo.setCompareEnable(false);
	// 	samplerInfo.setCompareOp(vk::CompareOp::eAlways);
	// 	samplerInfo.setMipmapMode(vk::SamplerMipmapMode::eLinear);
	// 	samplerInfo.setMipLodBias(0.0f);
	// 	samplerInfo.setMinLod(0.0f);
	// 	samplerInfo.setMaxLod(0.0f);
	// 
	// 	// Create sampler
	// 	m_sampler = m_graphics->getDeviceManager()->getLogicalDevice().createSampler(samplerInfo);
	// }

	Texture::~Texture()
	{
		vk::Device logicalDevice = m_graphics->getLogicalDevice();

		logicalDevice.destroySampler(m_sampler);
		logicalDevice.destroyImageView(m_imageView);
		logicalDevice.freeMemory(m_imageMemory);
		logicalDevice.destroyImage(m_image);
	}
}