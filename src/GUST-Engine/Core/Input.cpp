#include "Input.hpp"

namespace gust
{
	void Input::startup()
	{
		// Initialize SDL events
		SDL_Init(SDL_INIT_EVENTS);
	}

	void Input::shutdown()
	{

	}

	void Input::pollEvents()
	{
		// Set last key presses
		m_lastKeys = m_currentKeys;

		// Set last mouse buttons
		m_leftMouseButton = std::make_tuple(std::get<0>(m_leftMouseButton), std::get<0>(m_leftMouseButton));
		m_rightMouseButton = std::make_tuple(std::get<0>(m_rightMouseButton), std::get<0>(m_rightMouseButton));
		m_middleMouseButton = std::make_tuple(std::get<0>(m_middleMouseButton), std::get<0>(m_middleMouseButton));

		// Say we haven't found the last mouse delta
		m_foundMouseDelta = false;

		// Loop through events
		SDL_Event evt;

		while (SDL_PollEvent(&evt))
		{
			if (evt.type == SDL_QUIT)
				m_closing = true;

			// Get key down
			else if (evt.type == SDL_KEYDOWN)
				m_currentKeys[static_cast<KeyCode>(evt.key.keysym.sym)] = true;

			// Get key up
			else if (evt.type == SDL_KEYUP)
				m_currentKeys[static_cast<KeyCode>(evt.key.keysym.sym)] = false;

			// Get mouse button down
			else if (evt.type == SDL_MOUSEBUTTONDOWN)
			{
				if (evt.button.button == SDL_BUTTON_LEFT)
					m_leftMouseButton = std::make_tuple(true, std::get<1>(m_leftMouseButton));

				else if (evt.button.button == SDL_BUTTON_RIGHT)
					m_rightMouseButton = std::make_tuple(true, std::get<1>(m_rightMouseButton));

				else if (evt.button.button == SDL_BUTTON_MIDDLE)
					m_middleMouseButton = std::make_tuple(true, std::get<1>(m_middleMouseButton));
			}

			// Get mouse button up
			else if (evt.type == SDL_MOUSEBUTTONUP)
			{
				if (evt.button.button == SDL_BUTTON_LEFT)
					m_leftMouseButton = std::make_tuple(false, std::get<1>(m_leftMouseButton));

				else if (evt.button.button == SDL_BUTTON_RIGHT)
					m_rightMouseButton = std::make_tuple(false, std::get<1>(m_rightMouseButton));

				else if (evt.button.button == SDL_BUTTON_MIDDLE)
					m_middleMouseButton = std::make_tuple(false, std::get<1>(m_middleMouseButton));
			}
		}
	}



	void Input::registerButton(const std::string& name, KeyCode key)
	{
		m_buttons.insert(std::pair<std::string, KeyCode>(name, key));
	}

	void Input::registerAxis(const std::string& name, const KeyAxis& axis)
	{
		m_axis.insert(std::pair<std::string, KeyAxis>(name, axis));
	}
}