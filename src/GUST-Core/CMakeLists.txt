# Source files
set(
	GUST_CORE_SRCS
	Allocators.cpp
	Debugging.cpp
	Hashing.cpp
)
	
# Header files
set(
	GUST_CORE_HDRS
	Allocators.hpp
	Debugging.hpp
	Hashing.hpp
	Parsers.hpp
	Math.hpp
)

# Project Declaration
ADD_LIBRARY(
	GUST-Core 
	${GUST_CORE_SRCS} 
	${GUST_CORE_HDRS}
)