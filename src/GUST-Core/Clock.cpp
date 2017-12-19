#include "Clock.hpp"

namespace gust
{
	Clock::Clock()
	{
		m_creationTime = SDL_GetPerformanceCounter();
		m_measuringTime = m_creationTime;
	}
}