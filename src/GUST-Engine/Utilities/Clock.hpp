#pragma once

/**
 * @file Clock.hpp
 * @brief Clock header file.
 * @author Connor J. Bramham (ReeCocho)
 */

/** Includes. */
#include <SDL_timer.h>

namespace gust
{
	/**
	 * @class Clock
	 * @brief Allows for timing
	 */
	class Clock
	{
	public:

		/**
		 * @brief Constructor.
		 */
		Clock();

		/**
		 * @brief Destructor.
		 */
		~Clock() = default;

		/**
		 * @brief Get the time the clock was created.
		 * @return Time the clock was created.
		 * @note In seconds
		 */
		inline float getCreationTime() const
		{
			return	static_cast<float>(m_creationTime) / 
					static_cast<float>(SDL_GetPerformanceFrequency());
		}

		/**
		 * @brief Get elapsed time since the clock was created.
		 * @note In seconds.
		 */
		inline float getElapsedTime() const
		{
			return	static_cast<float>(SDL_GetPerformanceCounter() - m_creationTime) / 
					static_cast<float>(SDL_GetPerformanceFrequency());
		}

		/**
		 * @brief Get time since last measuring delta time.
		 * @return Time since last measuring delta time.
		 * @note In seconds.
		 */
		inline float getDeltaTime()
		{
			uint64_t newTime = SDL_GetPerformanceCounter();
			float deltaTime =	static_cast<float>(newTime - m_measuringTime) / 
								static_cast<float>(SDL_GetPerformanceFrequency());
			m_measuringTime = newTime;

			return deltaTime;
		}

	private:

		/** Time the clock was created. */
		uint64_t m_creationTime;

		/** Last time the delta was measured. */
		uint64_t m_measuringTime;
	};
}