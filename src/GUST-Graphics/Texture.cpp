#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <Debugging.hpp>
#include "Texture.hpp"

namespace gust
{
	Texture::Texture(Graphics* graphics, vk::Image image, vk::ImageView& imageView, vk::Sampler sampler, vk::DeviceMemory memory, uint32_t width, uint32_t height) :
		m_graphics(graphics),
		m_image(image),
		m_imageView(imageView),
		m_sampler(sampler),
		m_imageMemory(memory),
		m_width(width),
		m_height(height)
	{
		
	}

	Texture::Texture(Graphics* graphics, const std::string& path, vk::Filter filter) : m_graphics(graphics), m_filtering(filter)
	{
		auto logicalDevice = m_graphics->getLogicalDevice();

		// Load image from file
		int texWidth = 0, texHeight = 0, texChannels = 0;
		auto pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	
		m_width = static_cast<uint32_t>(texWidth);
		m_height = static_cast<uint32_t>(texHeight);
	
		gAssert(pixels);
	
		auto imageSize = static_cast<vk::DeviceSize>(m_width * m_height * 4);
	
		// Create staging buffer
		Buffer stagingBuffer = m_graphics->createBuffer
		(
			imageSize,
			vk::BufferUsageFlagBits::eTransferSrc,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
		);
	
		// Map image to memory
		void* data;
		logicalDevice.mapMemory(stagingBuffer.memory, 0, imageSize, (vk::MemoryMapFlagBits)0, &data);
		memcpy(data, pixels, static_cast<size_t>(imageSize));
		logicalDevice.unmapMemory(stagingBuffer.memory);
	
		// Free pixel data
		stbi_image_free(pixels);
	
		// Create image
		m_graphics->createImage
		(
			m_width,
			m_height,
			vk::Format::eR8G8B8A8Unorm,
			vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
			vk::MemoryPropertyFlagBits::eDeviceLocal,
			m_image,
			m_imageMemory
		);
	
		// Prepare texture for shader access
		m_graphics->transitionImageLayout(m_image, vk::Format::eR8G8B8A8Unorm, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
		m_graphics->copyBufferToImage(stagingBuffer.buffer, m_image, static_cast<uint32_t>(m_width), static_cast<uint32_t>(m_height));
		m_graphics->transitionImageLayout(m_image, vk::Format::eR8G8B8A8Unorm, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
	
		// Cleanup staging buffer
		logicalDevice.destroyBuffer(stagingBuffer.buffer);
		logicalDevice.freeMemory(stagingBuffer.memory);
	
		// Create texture image view
		m_imageView = m_graphics->createImageView(m_image, vk::Format::eR8G8B8A8Unorm, vk::ImageAspectFlagBits::eColor);
	
		// Sampler creation info
		vk::SamplerCreateInfo samplerInfo = {};
		samplerInfo.setMagFilter(filter);
		samplerInfo.setMinFilter(filter);
		samplerInfo.setAddressModeU(vk::SamplerAddressMode::eRepeat);
		samplerInfo.setAddressModeV(vk::SamplerAddressMode::eRepeat);
		samplerInfo.setAddressModeW(vk::SamplerAddressMode::eRepeat);
		samplerInfo.setAnisotropyEnable(true);
		samplerInfo.setMaxAnisotropy(1);
		samplerInfo.setBorderColor(vk::BorderColor::eFloatOpaqueBlack);
		samplerInfo.setUnnormalizedCoordinates(false);
		samplerInfo.setCompareEnable(false);
		samplerInfo.setCompareOp(vk::CompareOp::eAlways);
		samplerInfo.setMipmapMode(vk::SamplerMipmapMode::eLinear);
		samplerInfo.setMipLodBias(0.0f);
		samplerInfo.setMinLod(0.0f);
		samplerInfo.setMaxLod(0.0f);
	
		// Create sampler
		m_sampler = logicalDevice.createSampler(samplerInfo);
	}

	Texture::~Texture()
	{
		vk::Device logicalDevice = m_graphics->getLogicalDevice();

		if(m_sampler)
			logicalDevice.destroySampler(m_sampler);

		if(m_imageView)
			logicalDevice.destroyImageView(m_imageView);

		if(m_imageMemory)
			logicalDevice.freeMemory(m_imageMemory);

		if(m_image)
			logicalDevice.destroyImage(m_image);
	}



	Cubemap::Cubemap
	(
		Graphics* graphics,
		vk::Image image,
		vk::ImageView& imageView,
		vk::Sampler sampler,
		vk::DeviceMemory memory,
		uint32_t width,
		uint32_t height
	) : Texture(graphics, image, imageView, sampler, memory, width, height)
	{

	}

	Cubemap::Cubemap
	(
		Graphics* graphics,
		const std::string& top,
		const std::string& bottom,
		const std::string& north,
		const std::string& east,
		const std::string& south,
		const std::string& west,
		vk::Filter filter
	)
	{
		m_graphics = graphics;
		m_filtering = filter;

		auto logicalDevice = m_graphics->getLogicalDevice();

		// Load image from file
		int texWidth = 0, texHeight = 0, texChannels = 0;
		unsigned char* topPixels	= stbi_load(top.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		unsigned char* bottomPixels	= stbi_load(bottom.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		unsigned char* northPixels	= stbi_load(north.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		unsigned char* eastPixels	= stbi_load(east.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		unsigned char* southPixels	= stbi_load(south.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		unsigned char* westPixels	= stbi_load(west.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

		m_width = static_cast<uint32_t>(texWidth);
		m_height = static_cast<uint32_t>(texHeight);
	
		gAssert(topPixels && bottomPixels && northPixels && eastPixels && southPixels && westPixels);
	
		auto imageSize = static_cast<vk::DeviceSize>((m_width * m_height * 4) * 6);
		auto singleImageSize = static_cast<vk::DeviceSize>(m_width * m_height * 4);

		// Combined images
		unsigned char* total = new unsigned char[static_cast<size_t>(imageSize)];
		memcpy(total						, westPixels, singleImageSize);
		memcpy(total + singleImageSize		, eastPixels, singleImageSize);
		memcpy(total + (2 * singleImageSize), topPixels, singleImageSize);
		memcpy(total + (3 * singleImageSize), bottomPixels, singleImageSize);
		memcpy(total + (4 * singleImageSize), northPixels, singleImageSize);
		memcpy(total + (5 * singleImageSize), southPixels, singleImageSize);

		// Create staging buffer
		Buffer stagingBuffer = m_graphics->createBuffer
		(
			imageSize,
			vk::BufferUsageFlagBits::eTransferSrc,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
		);
	
		// Map image to memory
		void* data;
		logicalDevice.mapMemory(stagingBuffer.memory, 0, imageSize, (vk::MemoryMapFlagBits)0, &data);
		memcpy(data, total, static_cast<size_t>(imageSize));
		logicalDevice.unmapMemory(stagingBuffer.memory);
	
		// Free pixel data
		stbi_image_free(topPixels);
		stbi_image_free(bottomPixels);
		stbi_image_free(northPixels);
		stbi_image_free(eastPixels);
		stbi_image_free(southPixels);
		stbi_image_free(westPixels);
		delete total;
	
		// Create image
		m_graphics->createImage
		(
			m_width,
			m_height,
			vk::Format::eR8G8B8A8Unorm,
			vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
			vk::MemoryPropertyFlagBits::eDeviceLocal,
			m_image,
			m_imageMemory,
			vk::ImageCreateFlagBits::eCubeCompatible,
			6
		);
	
		// Prepare texture for shader access
		m_graphics->transitionImageLayout(m_image, vk::Format::eR8G8B8A8Unorm, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, 6);
		m_graphics->copyBufferToImage(stagingBuffer.buffer, m_image, static_cast<uint32_t>(m_width), static_cast<uint32_t>(m_height), 6, static_cast<uint32_t>(singleImageSize));
		m_graphics->transitionImageLayout(m_image, vk::Format::eR8G8B8A8Unorm, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, 6);
	
		// Cleanup staging buffer
		logicalDevice.destroyBuffer(stagingBuffer.buffer);
		logicalDevice.freeMemory(stagingBuffer.memory);
	
		// Create texture image view
		m_imageView = m_graphics->createImageView(m_image, vk::Format::eR8G8B8A8Unorm, vk::ImageAspectFlagBits::eColor, vk::ImageViewType::eCube, 6);
	
		// Sampler creation info
		vk::SamplerCreateInfo samplerInfo = {};
		samplerInfo.setMagFilter(filter);
		samplerInfo.setMinFilter(filter);
		samplerInfo.setAddressModeU(vk::SamplerAddressMode::eRepeat);
		samplerInfo.setAddressModeV(vk::SamplerAddressMode::eRepeat);
		samplerInfo.setAddressModeW(vk::SamplerAddressMode::eRepeat);
		samplerInfo.setAnisotropyEnable(true);
		samplerInfo.setMaxAnisotropy(1);
		samplerInfo.setBorderColor(vk::BorderColor::eFloatOpaqueBlack);
		samplerInfo.setUnnormalizedCoordinates(false);
		samplerInfo.setCompareEnable(false);
		samplerInfo.setCompareOp(vk::CompareOp::eAlways);
		samplerInfo.setMipmapMode(vk::SamplerMipmapMode::eLinear);
		samplerInfo.setMipLodBias(0.0f);
		samplerInfo.setMinLod(0.0f);
		samplerInfo.setMaxLod(0.0f);
	
		// Create sampler
		m_sampler = logicalDevice.createSampler(samplerInfo);
	}
}