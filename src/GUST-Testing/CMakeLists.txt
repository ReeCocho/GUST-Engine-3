# Source Files
set(
	GUST_TESTING_SRCS 
	Main.cpp
)
	
# Header files
set(
	GUST_TESTING_HDRS
)

# Executable
add_executable (
	GUST-Testing 
	${GUST_TESTING_SRCS} 
	${GUST_TESTING_HDRS}
)

# Includes
include_directories(${CMAKE_SOURCE_DIR}/src/GUST-Engine)
include_directories(${CMAKE_SOURCE_DIR}/src/GUST-ECS)
include_directories(${CMAKE_SOURCE_DIR}/src/GUST-Core)
include_directories(${CMAKE_SOURCE_DIR}/src/GUST-Graphics)
include_directories(${CMAKE_SOURCE_DIR}/src/GUST-Physics)

# GUST Libraries
SET(
	LINK_LIBRARY 
	optimized 
	${CMAKE_SOURCE_DIR}/build/src/GUST-Engine/Release/GUST-Engine.lib
	${CMAKE_SOURCE_DIR}/build/src/GUST-Graphics/Release/GUST-Graphics.lib
	${CMAKE_SOURCE_DIR}/build/src/GUST-Core/Release/GUST-Core.lib
	${CMAKE_SOURCE_DIR}/build/src/GUST-ECS/Release/GUST-ECS.lib
	${CMAKE_SOURCE_DIR}/build/src/GUST-Physics/Release/GUST-Physics.lib
	debug 
	${CMAKE_SOURCE_DIR}/build/src/GUST-Engine/Debug/GUST-Engine.lib
	${CMAKE_SOURCE_DIR}/build/src/GUST-Graphics/Debug/GUST-Graphics.lib
	${CMAKE_SOURCE_DIR}/build/src/GUST-Core/Debug/GUST-Core.lib
	${CMAKE_SOURCE_DIR}/build/src/GUST-ECS/Debug/GUST-ECS.lib
	${CMAKE_SOURCE_DIR}/build/src/GUST-Physics/Debug/GUST-Physics.lib
)

# Libraries
target_link_libraries(
	GUST-Testing 
	GUST-Engine
	GUST-Graphics
	GUST-Core
	GUST-ECS
	GUST-Physics
	${SDL2_LIBRARY} 
	${VULKAN_LIBRARY}
	${Bullet_LIBRARIES}
)

# Move shader folder and mesh into build directory
add_custom_command(
	TARGET GUST-Testing POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	${CMAKE_SOURCE_DIR}/src/Shaders/Output $<TARGET_FILE_DIR:GUST-Testing>/../Shaders
)

add_custom_command(
	TARGET GUST-Testing POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	${CMAKE_SOURCE_DIR}/src/Meshes $<TARGET_FILE_DIR:GUST-Testing>/../Meshes
)