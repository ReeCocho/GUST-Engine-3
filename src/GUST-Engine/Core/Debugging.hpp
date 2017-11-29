#pragma once

/**
 * @file Debugging.hpp
 * @brief Debugging header file.
 * @author Connor J. Bramham (ReeCocho)
 */

/** Includes. */
#include <string>
#include <assert.h>

namespace gust
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

	/**
	 * @brief Throw an error and stop the application.
	 * @param Error message.
	 */
	extern void throwError(const std::string& msg);
}