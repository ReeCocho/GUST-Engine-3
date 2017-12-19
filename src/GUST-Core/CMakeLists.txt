# Sources
set(
	GUST_CORE_SRCS
	Allocators.cpp
	Clock.cpp
	Debugging.cpp
	FileIO.cpp
	Hashing.cpp
	Threading.cpp
)

# Headers
set(
	GUST_CORE_HDRS
	Allocators.hpp
	Clock.hpp
	Debugging.hpp
	FileIO.hpp
	Hashing.hpp
	Math.hpp
	Parsers.hpp
	Threading.hpp
)

# Project declaration
ADD_LIBRARY(
	GUST-Core
	${GUST_CORE_SRCS}
	${GUST_CORE_HDRS}
)