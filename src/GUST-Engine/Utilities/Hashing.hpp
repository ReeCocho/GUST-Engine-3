#pragma once

/**
 * @file Hashing.hpp
 * @brief Hashing header file.
 * @author Connor J. Bramham (ReeCocho)
 */

/** Includes. */
#include <string>

namespace gust
{
	/**
	 * @brief Hash a string.
	 * @param String to hash.
	 * @return Hashed string.
	 */
	extern size_t hash(const std::string& str);
}