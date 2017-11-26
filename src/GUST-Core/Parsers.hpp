#pragma once

/**
 * @file Parsers.hpp
 * @brief Parsers header file.
 * @author Connor J. Bramham (ReeCocho)
 */

/** Includes. */
#include <json.hpp>

namespace gust
{
	namespace core
	{
		// JSON library from nlohamnn
		using json = nlohmann::json;
	}
}