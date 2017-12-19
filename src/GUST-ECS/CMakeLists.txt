# Sources
set(
	GUST_ECS_SRCS
	Component.cpp
	Entity.cpp
	Scene.cpp
	System.cpp
	Transform.cpp
)

# Headers
set(
	GUST_ECS_HDRS
	Component.hpp
	Entity.hpp
	Scene.hpp
	System.hpp
	Transform.hpp
)

# Project declaration
ADD_LIBRARY(
	GUST-ECS
	${GUST_ECS_SRCS}
	${GUST_ECS_HDRS}
)

# Includes
include_directories(../GUST-Core)

# Dependencies
add_dependencies(
	GUST-ECS
	GUST-Core
)