#include <Debugging.hpp>
#include "VulkanDebugging.hpp"

namespace gust
{
	VulkanDebugging::VulkanDebugging(vk::Instance instance) : m_instance(instance)
	{
		VkDebugReportCallbackCreateInfoEXT createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
		createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;		// Debug flags
		createInfo.pfnCallback = debugCallback;													// Debug function

		// Create debug function
		auto func = (PFN_vkCreateDebugReportCallbackEXT)m_instance.getProcAddr("vkCreateDebugReportCallbackEXT");
		assert(func != nullptr);

		// Create debug callback
		VkDebugReportCallbackEXT callback = VK_NULL_HANDLE;
		func(static_cast<VkInstance>(m_instance), &createInfo, nullptr, &callback);

		// Convert to C++ Vulkan
		m_debugCallback = vk::DebugReportCallbackEXT(callback);
	}

	VulkanDebugging::~VulkanDebugging()
	{
		// Destroy debug callback
		auto func = (PFN_vkDestroyDebugReportCallbackEXT)m_instance.getProcAddr("vkDestroyDebugReportCallbackEXT");
		assert(func);
		func(static_cast<VkInstance>(m_instance), static_cast<VkDebugReportCallbackEXT>(m_debugCallback), nullptr);
	}

	VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugging::debugCallback
	(
		VkDebugReportFlagsEXT flags,
		VkDebugReportObjectTypeEXT objType,
		uint64_t obj,
		size_t location,
		int32_t code,
		const char* layerPrefix,
		const char* msg,
		void* userData
	)
	{
		gLog("Vulkan Validation Layer: " << msg << '\n');
		return VK_FALSE;
	}
}