#include <iostream>
#include "Debugging.hpp"

namespace gust
{
	namespace core
	{
		void print(const std::string& str)
		{
#ifndef NDEBUG
			std::cout << str;
#endif
		}

		void printLine(const std::string& str)
		{
#ifndef NDEBUG
			std::cout << str << '\n';
#endif
		}
	}
}