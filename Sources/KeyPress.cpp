#include "KeyPress.hpp"

namespace API_NAME {
	KeyPress::KeyPress(Win32KeyCode const keycode) noexcept : m_KeyCode{ keycode }, m_Start{ 0 } {

	}

	void KeyPress::press() noexcept {
		m_Start = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch());
	}
	void KeyPress::release() noexcept {
		m_Start = std::chrono::nanoseconds{ 0 };
	}

	std::chrono::nanoseconds KeyPress::getDuration() noexcept {
		if (m_Start == std::chrono::nanoseconds{ 0 }) return std::chrono::nanoseconds{ 0 };
		return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()) - m_Start;
	}
	bool KeyPress::isPressed() const noexcept {
		return m_Start != std::chrono::nanoseconds{ 0 };
	}

	Win32KeyCode KeyPress::keycode() const noexcept {
		return m_KeyCode;
	}
}