# Sources
set(
	GUST_PHYSICS_SRCS
	Physics.cpp
)

# Headers
set(
	GUST_PHYSICS_HDRS
	Physics.hpp
)

# Project declaration
ADD_LIBRARY(
	GUST-Physics
	${GUST_PHYSICS_SRCS}
	${GUST_PHYSICS_HDRS}
)

# Includes
include_directories(../GUST-Core)

# Dependencies
add_dependencies(
	GUST-Physics
	GUST-Core
)