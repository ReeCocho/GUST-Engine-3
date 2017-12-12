# Core
file(
	GLOB_RECURSE 
	GUST_CORE
	"${CMAKE_SOURCE_DIR}/src/GUST-Engine/Core/*.cpp"
	"${CMAKE_SOURCE_DIR}/src/GUST-Engine/Core/*.hpp"
)

# ECS
file(
	GLOB_RECURSE 
	GUST_ECS
	"${CMAKE_SOURCE_DIR}/src/GUST-Engine/ECS/*.cpp"
	"${CMAKE_SOURCE_DIR}/src/GUST-Engine/ECS/*.hpp"
)

# Graphics
file(
	GLOB_RECURSE 
	GUST_GRAPHICS
	"${CMAKE_SOURCE_DIR}/src/GUST-Engine/Graphics/*.cpp"
	"${CMAKE_SOURCE_DIR}/src/GUST-Engine/Graphics/*.hpp"
)

# Utilities
file(
	GLOB_RECURSE 
	GUST_UTILITIES
	"${CMAKE_SOURCE_DIR}/src/GUST-Engine/Utilities/*.cpp"
	"${CMAKE_SOURCE_DIR}/src/GUST-Engine/Utilities/*.hpp"
)

# Sources
set(
	GUST_ENGINE_SRCS
	Engine.cpp
)

# Headers
set(
	GUST_ENGINE_HDRS
	Engine.hpp
)

# Project Declaration
ADD_LIBRARY(
	GUST-Engine
	${GUST_CORE} 
	${GUST_GRAPHICS}
	${GUST_UTILITIES}
	${GUST_ECS}
	${GUST_ENGINE_SRCS}
	${GUST_ENGINE_HDRS}
) 

# Grouping
source_group("Core" FILES ${GUST_CORE})
source_group("Utilites" FILES ${GUST_UTILITIES})
source_group("Graphics" FILES ${GUST_GRAPHICS})
source_group("ECS" FILES ${GUST_ECS})