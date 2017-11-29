#pragma once

/**
 * @file Input.hpp
 * @brief Input header file.
 * @author Connor J. Bramham (ReeCocho)
 */

 /** Includes. */
#include <SDL.h>
#include <map>
#include <vector>
#include <string>
#include <tuple>
#include <glm\glm.hpp>

namespace gust
{
	/**
	 * @enum MouseButton
	 * @brief Mouse button enumeration.
	 */
	enum class MouseButton
	{
		Left = 0,
		Right = 1,
		Middle = 2
	};

	/**
	 * @enum KeyCode
	 * @brief Keycodes.
	 */
	enum class KeyCode
	{
		Unkown = 0,

		Return = '\r',
		Escape = '\033',
		Backspace = '\b',
		Tab = '\t',
		Space = ' ',
		Exclaimation = '!',
		DoubleQuote = '"',
		Hash = '#',
		Percent = '%',
		Dollar = '$',
		Ampersand = '&',
		Quote = '\'',
		LeftParen = '(',
		RightParent = ')',
		Asterisk = '*',
		Plus = '+',
		Comma = ',',
		Minus = '-',
		Period = '.',
		Slash = '/',
		Zero = '0',
		One = '1',
		Two = '2',
		Three = '3',
		Four = '4',
		Five = '5',
		Six = '6',
		Seven = '7',
		Eight = '8',
		Nine = '9',
		Colon = ':',
		Semicolon = ';',
		Less = '<',
		Equals = '=',
		Greater = '>',
		Question = '?',
		At = '@',

		LeftBracket = '[',
		Backslash = '\\',
		RightBracket = ']',
		Caret = '^',
		Underscore = '_',
		Tilde = '`',
		A = 'a',
		B = 'b',
		C = 'c',
		D = 'd',
		E = 'e',
		F = 'f',
		G = 'g',
		H = 'h',
		I = 'i',
		J = 'j',
		K = 'k',
		L = 'l',
		M = 'm',
		N = 'n',
		O = 'o',
		P = 'p',
		Q = 'q',
		R = 'r',
		S = 's',
		T = 't',
		U = 'u',
		V = 'v',
		W = 'w',
		X = 'x',
		Y = 'y',
		Z = 'z',

		CaspsLock = (57 | (1 << 30)),

		F1 = (58 | (1 << 30)),
		F2 = (59 | (1 << 30)),
		F3 = (60 | (1 << 30)),
		F4 = (61 | (1 << 30)),
		F5 = (62 | (1 << 30)),
		F6 = (63 | (1 << 30)),
		F7 = (64 | (1 << 30)),
		F8 = (65 | (1 << 30)),
		F9 = (66 | (1 << 30)),
		F10 = (67 | (1 << 30)),
		F11 = (68 | (1 << 30)),
		F12 = (69 | (1 << 30)),
		F13 = (104 | (1 << 30)),
		F14 = (105 | (1 << 30)),
		F15 = (106 | (1 << 30)),
		F16 = (107 | (1 << 30)),
		F17 = (108 | (1 << 30)),
		F18 = (109 | (1 << 30)),
		F19 = (110 | (1 << 30)),
		F20 = (111 | (1 << 30)),
		F21 = (112 | (1 << 30)),
		F22 = (113 | (1 << 30)),
		F23 = (114 | (1 << 30)),
		F24 = (115 | (1 << 30)),

		PrintScreen = (70 | (1 << 30)),
		ScrollLock = (71 | (1 << 30)),
		Pause = (72 | (1 << 30)),
		Insert = (73 | (1 << 30)),
		Home = (74 | (1 << 30)),
		PageUp = (75 | (1 << 30)),
		Delete = '\177',
		End = (77 | (1 << 30)),
		PageDown = (78 | (1 << 30)),
		Right = (79 | (1 << 30)),
		Left = (80 | (1 << 30)),
		Down = (81 | (1 << 30)),
		Up = (82 | (1 << 30)),

		NumLockClear = (83 | (1 << 30)),

		LeftControl = (224 | (1 << 30)),
		LeftShift = (225 | (1 << 30)),
		LeftAlt = (226 | (1 << 30)),
		RightControl = (228 | (1 << 30)),
		RightShift = (229 | (1 << 30)),
		RightAlt = (230 | (1 << 30))
	};

	/**
	 * @typedef KeyAxis
	 * @brief List of key codes and their corosponding values.
	 */
	typedef std::vector<std::tuple<KeyCode, float>> KeyAxis;

	/**
	 * @class Input
	 * @brief Input manager.
	 */
	class Input
	{
	public:

		/**
		 * @brief Default constructor.
		 */
		Input() = default;

		/**
		 * @brief Default destructor.
		 */
		~Input() = default;



		/**
		 * @brief Startup input.
		 * @note This is called internally. Do not use.
		 */
		void startup();

		/**
		 * @brief Shutdown input.
		 * @note This is called internally. Do not use.
		 */
		void shutdown();

		/**
		 * @brief Poll input events.
		 */
		void pollEvents();

		/**
		 * @brief Lock/unlock the mouse in the window.
		 * @param New mouse locking state.
		 * @return New mouse locking state.
		 */
		inline bool setLockedMouse(bool state) const
		{
			SDL_SetRelativeMouseMode(state ? SDL_TRUE : SDL_FALSE);
			return state;
		}

		/**
		 * @brief Get if the key just pressed.
		 * @param Key code.
		 * @return Was the key just pressed?
		 */
		inline bool getKeyDown(KeyCode key) const
		{
			return !m_lastKeys.at(key) && m_currentKeys.at(key);
		}

		/**
		 * @brief Get if the key just released.
		 * @param Key code.
		 * @return Was the key just released?
		 */
		inline bool getKeyUp(KeyCode key) const
		{
			return m_lastKeys.at(key) && !m_currentKeys.at(key);
		}

		/**
		 * @brief Get if the key is being helf.
		 * @param Key code.
		 * @return Is the key being held?
		 */
		inline bool getKeyHeld(KeyCode key) const
		{
			return m_currentKeys.at(key);
		}

		/**
		 * @brief Get if the button just pressed.
		 * @param Button name.
		 * @return Was the button just pressed?
		 */
		inline bool getButtonDown(const std::string& button) const
		{
			return getKeyDown(m_buttons.at(button));
		}

		/**
		 * @brief Get if the button just released.
		 * @param Button name.
		 * @return Was the button just released?
		 */
		inline bool getButtonUp(const std::string& button) const
		{
			return getKeyUp(m_buttons.at(button));
		}

		/**
		 * @brief Get if the button is being held.
		 * @param Button name.
		 * @return Is the button being held?
		 */
		inline bool getButtonHeld(const std::string& button) const
		{
			return getKeyHeld(m_buttons.at(button));
		}

		/**
		 * @brief Get if the axis was just pressed.
		 * @param Axis name.
		 * @return Was the axis just pressed?
		 */
		inline bool getAxisDown(const std::string& axis) const
		{
			for (size_t i = 0; i < m_axis.at(axis).size(); i++)
				if (getKeyDown(std::get<0>(m_axis.at(axis)[i])))
					return true;

			return false;
		}

		/**
		 * @brief Get if the axis was just released.
		 * @param Axis name.
		 * @return Was the axis just released?
		 */
		inline bool getAxisUp(const std::string& axis) const
		{
			for (size_t i = 0; i < m_axis.at(axis).size(); i++)
				if (getKeyUp(std::get<0>(m_axis.at(axis)[i])))
					return true;

			return false;
		}

		/**
		 * @brief Get if the axis is being used.
		 * @param Axis name.
		 * @return Is the axis being used.
		 */
		inline bool getAxisHeld(const std::string& axis) const
		{
			for (size_t i = 0; i < m_axis.at(axis).size(); i++)
				if (getKeyHeld(std::get<0>(m_axis.at(axis)[i])))
					return true;

			return false;
		}

		/**
		 * @brief Get the axis' current value.
		 * @param Axis name.
		 * @return Axis' value.
		 */
		inline float getAxis(const std::string& axis) const
		{
			float total = 0;

			for (size_t i = 0; i < m_axis.at(axis).size(); i++)
				if (getKeyHeld(std::get<0>(m_axis.at(axis)[i])))
					total += std::get<1>(m_axis.at(axis)[i]);

			return total;
		}

		/**
		 * @brief Get change in mouse position.
		 * @return Mouse delta
		 */
		inline glm::vec2 getMouseDelta()
		{
			if (!m_foundMouseDelta)
			{
				int x = 0;
				int y = 0;
			
				SDL_GetRelativeMouseState(&x, &y);
				m_mouseDelta = glm::vec2(x, y);
				m_foundMouseDelta = true;
			}

			return m_mouseDelta;
		}

		/**
		 * @brief Get mouses position.
		 * @return Mouses position.
		 */
		inline glm::vec2 getMousePosition() const
		{
			int x = 0;
			int y = 0;

			SDL_GetMouseState(&x, &y);
			return glm::vec2(x, y);
		}

		/**
		 * @brief Get if the mouse button was just pressed.
		 * @param Mouse button.
		 * @return Was the mouse button just pressed?
		 */
		inline bool getMouseButtonDown(MouseButton button) const
		{
			switch (button)
			{
			case MouseButton::Left:
				return std::get<0>(m_leftMouseButton) && !std::get<1>(m_leftMouseButton);
			case MouseButton::Right:
				return std::get<0>(m_rightMouseButton) && !std::get<1>(m_rightMouseButton);
			case MouseButton::Middle:
				return std::get<0>(m_middleMouseButton) && !std::get<1>(m_middleMouseButton);
			}

			return false;
		}

		/**
		 * @brief Get if the mouse button was just released.
		 * @param Mouse button.
		 * @return Was the mouse button just released?
		 */
		inline bool getMouseButtonUp(MouseButton button) const
		{
			switch (button)
			{
			case MouseButton::Left:
				return !std::get<0>(m_leftMouseButton) && std::get<1>(m_leftMouseButton);
			case MouseButton::Right:
				return !std::get<0>(m_rightMouseButton) && std::get<1>(m_rightMouseButton);
			case MouseButton::Middle:
				return !std::get<0>(m_middleMouseButton) && std::get<1>(m_middleMouseButton);
			}

			return false;
		}

		/**
		 * @brief Get if the mouse button is being held.
		 * @param Mouse button.
		 * @return Is the mouse button being held?
		 */
		inline bool getMouseButtonHeld(MouseButton button) const
		{
			switch (button)
			{
			case MouseButton::Left:
				return std::get<0>(m_leftMouseButton);
			case MouseButton::Right:
				return std::get<0>(m_rightMouseButton);
			case MouseButton::Middle:
				return std::get<0>(m_middleMouseButton);
			}

			return false;
		}

		/**
		 * @brief Register a button.
		 * @param Name of the button.
		 * @param Button keycode.
		 */
		void registerButton(const std::string& name, KeyCode key);

		/**
		 * @brief Register an axis.
		 * @param Name of the axis.
		 * @param Axis data.
		 */
		void registerAxis(const std::string& name, const KeyAxis& axis);

		/**
		 * @brief Get if the window is closing.
		 * @return Is the window closing?
		 */
		inline bool isClosing() const
		{
			return m_closing;
		}

	private:

		/** Window quit flag. */
		bool m_closing = false;

		/** LMB State. */
		std::tuple<bool, bool> m_leftMouseButton = {};

		/** RMB State. */
		std::tuple<bool, bool> m_rightMouseButton = {};

		/** MMB State. */
		std::tuple<bool, bool> m_middleMouseButton = {};

		/** Current key states. */
		std::map<KeyCode, bool> m_currentKeys = {};

		/** Last key states. */
		std::map<KeyCode, bool> m_lastKeys = {};

		/** Buttons. */
		std::map<std::string, KeyCode> m_buttons = {};

		/** Axis'. */
		std::map<std::string, KeyAxis> m_axis = {};

		/** Have we gathered the mouse delta since the last pollEvents() call? */
		bool m_foundMouseDelta = false;

		/** Mouse position delta. */
		glm::vec2 m_mouseDelta = {};
	};
}