# Sources
set(
	GUST_ENGINE_SRCS
	Engine.cpp
	Camera.cpp
	Input.cpp
	Lights.cpp
	MeshRenderer.cpp
	ResourceManager.cpp
	Colliders.cpp
)

# Headers
set(
	GUST_ENGINE_HDRS
	Engine.hpp
	Camera.hpp
	Input.hpp
	Lights.hpp
	MeshRenderer.hpp
	ResourceManager.hpp
	Colliders.hpp
)

# Project declaration
ADD_LIBRARY(
	GUST-Engine
	${GUST_ENGINE_SRCS}
	${GUST_ENGINE_HDRS}
)

# Includes
include_directories(../GUST-Graphics)
include_directories(../GUST-ECS)
include_directories(../GUST-Core)
include_directories(../GUST-Physics)

# Dependencies
add_dependencies(
	GUST-Engine
	GUST-Graphics
	GUST-ECS
	GUST-Core
	GUST-Physics
)