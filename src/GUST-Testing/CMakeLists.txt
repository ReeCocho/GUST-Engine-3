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
	${CMAKE_SOURCE_DIR}/build/src/GUST-Core/Release/GUST-Core.lib
	debug 
	${CMAKE_SOURCE_DIR}/build/src/GUST-Core/Debug/GUST-Core.lib
)

# Libraries
target_link_libraries(
	GUST-Testing 
	${SDL2_LIBRARY} 
	${VULKAN_LIBRARY}
	${LINK_LIBRARY}
)