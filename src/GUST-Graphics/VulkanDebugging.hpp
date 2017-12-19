#pragma once

/**
 * @file VulkanDebugging.hpp
 * @brief Vulkan debugging header file
 * @author Connor J. Bramham (ReeCocho)
 */

/** Includes. */
#include <vulkan\vulkan.hpp>

namespace gust
{
	/**
	 * @class VulkanDebugging
	 * @brief Allows for debugging a Vulkan instance.
	 */
	class VulkanDebugging
	{
	public:

		/**
		 * @brief Default constructor.
		 */
		VulkanDebugging() = default;

		/**
		 * @brief Constructor.
		 * @param Instance to debug.
		 */
		VulkanDebugging(vk::Instance instance);

		/**
		 * @brief Destructor.
		 */
		~VulkanDebugging();

	private:

		/**
		 * @brief Vulkan debugging callback.
		 */
		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback
		(
			VkDebugReportFlagsEXT flags,
			VkDebugReportObjectTypeEXT objType,
			uint64_t obj,
			size_t location,
			int32_t code,
			const char* layerPrefix,
			const char* msg,
			void* userData
		);



		/** Debug reporting callback. */
		vk::DebugReportCallbackEXT m_debugCallback;

		/** Vulkan instance. */
		const vk::Instance m_instance;
	};
}