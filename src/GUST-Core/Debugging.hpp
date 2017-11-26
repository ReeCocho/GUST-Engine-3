#pragma once

/**
 * @file Debugging.hpp
 * @brief Debugging header file.
 * @author Connor J. Bramham (ReeCocho)
 */

/** Includes. */
#include <string>

namespace gust
{
	namespace core
	{
		/**
		 * @brief Prints a statement into the console and logs it.
		 * @param String to print and log.
		 */
		extern void print(const std::string& str);

		/**
		* @brief Prints a statement into the console on a new line and logs it.
		* @param String to print and log.
		*/
		extern void printLine(const std::string& str);
	}
}