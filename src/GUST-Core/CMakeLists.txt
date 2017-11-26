# Source files
set(
	GUST_CORE_SRCS
	Allocators.cpp
	Debugging.cpp
	Parsers.cpp
)
	
# Header files
set(
	GUST_CORE_HDRS
	Allocators.hpp
	Debugging.hpp
	Parsers.hpp
)

# Project Declaration
ADD_LIBRARY(
	GUST-Core 
	${GUST_CORE_SRCS} 
	${GUST_CORE_HDRS}
)