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
include_directories(${CMAKE_SOURCE_DIR}/src)

# GUST Libraries
SET(
	LINK_LIBRARY 
	optimized 
	${CMAKE_SOURCE_DIR}/build/src/GUST-Engine/Release/GUST-Engine.lib
	debug 
	${CMAKE_SOURCE_DIR}/build/src/GUST-Engine/Debug/GUST-Engine.lib
)

# Libraries
target_link_libraries(
	GUST-Testing 
	GUST-Engine
	${SDL2_LIBRARY} 
	${VULKAN_LIBRARY}
	# ${LINK_LIBRARY}
)

# Move shader folder into build directory
add_custom_command(
	TARGET GUST-Testing POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	${CMAKE_SOURCE_DIR}/src/Shaders/Output $<TARGET_FILE_DIR:GUST-Testing>/Shaders
)