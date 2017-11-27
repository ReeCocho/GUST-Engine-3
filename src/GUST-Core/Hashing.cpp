#include <xhash>
#include "Hashing.hpp"

namespace gust
{
	namespace core
	{
		size_t hash(const std::string& str)
		{
			std::hash<std::string> hasher;
			return hasher(str);
		}
	}
}