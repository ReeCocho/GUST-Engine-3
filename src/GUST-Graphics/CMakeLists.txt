# Sources
set(
	GUST_GRAPHICS_SRCS
	Graphics.cpp
	Material.cpp
	Mesh.cpp
	Renderer.cpp
	Shader.cpp
	Texture.cpp
	VulkanDebugging.cpp
)

# Headers
set(
	GUST_GRAPHICS_HDRS
	Graphics.hpp
	Material.hpp
	Mesh.hpp
	Renderer.hpp
	Shader.hpp
	Texture.hpp
	VulkanDebugging.hpp
	Vulkan.hpp
)

# Project declaration
ADD_LIBRARY(
	GUST-Graphics
	${GUST_GRAPHICS_SRCS}
	${GUST_GRAPHICS_HDRS}
)

# Includes
include_directories(../GUST-Core)

# Dependencies
add_dependencies(
	GUST-Graphics
	GUST-Core
)