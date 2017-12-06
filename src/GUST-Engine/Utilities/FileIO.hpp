#pragma once

/**
 * @file FileIO.hpp
 * @brief File IO header file.
 * @author Connor J. Bramham (ReeCocho)
 */

/** Includes. */
#include <string>
#include <vector>

namespace gust
{
	/**
	 * @brief Read a text file.
	 * @param Path to file.
	 * @return String containing the contents.
	 */
	extern std::string readText(const std::string& path);

	/**
	 * @brief Write a string to a text file.
	 * @param Path to file.
	 * @param String to write.
	 */
	extern void writeText(const std::string& path, const std::string& str);

	/**
	 * @brief Read a binary data file.
	 * @param Path to file.
	 * @return Vector containing the contents.
	 */
	extern std::vector<char> readBinary(const std::string& path);

	/**
	 * @brief Write binary data to a file.
	 * @param Path to file.
	 * @param Binary data to write.
	 */
	extern void writeBinary(const std::string& path, const std::vector<char>& bytes);
}