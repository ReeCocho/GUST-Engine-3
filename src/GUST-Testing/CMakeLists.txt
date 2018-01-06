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

# Libraries
target_link_libraries(
	GUST-Testing 
	${SDL2_LIBRARY} 
	${VULKAN_LIBRARY}
	${Bullet_LIBRARIES}
	GUST-Core
	GUST-Graphics
	GUST-ECS
	GUST-Physics
	GUST-Engine
)

# Move shader, mesh, scene, and texture folders into the build directory
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

add_custom_command(
	TARGET GUST-Testing POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	${CMAKE_SOURCE_DIR}/src/Textures $<TARGET_FILE_DIR:GUST-Testing>/../Textures
)

add_custom_command(
	TARGET GUST-Testing POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	${CMAKE_SOURCE_DIR}/src/Scenes $<TARGET_FILE_DIR:GUST-Testing>/../Scenes
)