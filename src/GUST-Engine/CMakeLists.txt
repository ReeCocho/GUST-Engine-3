# Source files
set(
	GUST_ENGINE_SRCS
	Allocators.cpp
	Debugging.cpp
	Hashing.cpp
	FileIO.cpp
	Clock.cpp
	ResourceManager.cpp
	Mesh.cpp
	Shader.cpp
	Material.cpp
	Texture.cpp
)
	
# Header files
set(
	GUST_ENGINE_HDRS
	Allocators.hpp
	Debugging.hpp
	Hashing.hpp
	FileIO.hpp
	Clock.hpp
	Parsers.hpp
	Math.hpp
	Vulkan.hpp
	ResourceManager.hpp
	Mesh.hpp
	Shader.hpp
	Material.hpp
	Texture.hpp
)

# Project Declaration
ADD_LIBRARY(
	GUST-Engine
	${GUST_ENGINE_SRCS} 
	${GUST_ENGINE_HDRS}
)