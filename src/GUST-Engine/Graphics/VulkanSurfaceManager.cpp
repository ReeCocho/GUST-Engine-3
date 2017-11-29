#include "VulkanSurfaceManager.hpp"

namespace gust
{
	VulkanSurfaceManager::VulkanSurfaceManager
	(
		vk::Instance instance,
		SDL_Window* window
	) :
		m_instance(instance)
	{
		// Create Vulkan surface.
		VkSurfaceKHR surface = VK_NULL_HANDLE;
		SDL_Vulkan_CreateSurface(window, static_cast<VkInstance>(m_instance), &surface);

		assert(surface != VK_NULL_HANDLE);

		m_surface = static_cast<vk::SurfaceKHR>(surface);
	}

	VulkanSurfaceManager::~VulkanSurfaceManager()
	{
		// Destroy surface
		m_instance.destroySurfaceKHR(m_surface);
	}

	void VulkanSurfaceManager::initSurfaceFormats(vk::PhysicalDevice physicalDevice)
	{
		// Check to see if we can display RGB colors
		const auto surfaceFormats = physicalDevice.getSurfaceFormatsKHR(m_surface);

		if (surfaceFormats.size() == 1 && surfaceFormats[0].format == vk::Format::eUndefined)
			m_colorFormat = vk::Format::eB8G8R8A8Unorm;
		else
			m_colorFormat = surfaceFormats[0].format;

		m_colorSpace = surfaceFormats[0].colorSpace;

		const auto formatProperties = physicalDevice.getFormatProperties(vk::Format::eR8G8B8A8Unorm);

		// Find a suitable depth format to use, starting with the best format
		std::vector<vk::Format> depthFormats =
		{
			vk::Format::eD32SfloatS8Uint,
			vk::Format::eD32Sfloat,
			vk::Format::eD24UnormS8Uint,
			vk::Format::eD16UnormS8Uint,
			vk::Format::eD16Unorm
		};

		for (const auto format : depthFormats)
		{
			const auto depthFormatProperties = physicalDevice.getFormatProperties(format);

			// Format must support depth stencil attachment for optimal tiling
			if (depthFormatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment)
			{
				m_depthFormat = format;
				break;
			}
		}
	}
}