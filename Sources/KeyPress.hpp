#pragma once

#include "Common/EnumStrings.hpp"

#pragma warning(push, 0)
#pragma warning(pop)

namespace API_NAME {
	class KeyPress {
	public:
		KeyPress() = default;
		KeyPress(Win32KeyCode const key_code) noexcept;

		void press() noexcept;
		void release() noexcept;

		std::chrono::nanoseconds getDuration() noexcept;
		bool isPressed() const noexcept;


		Win32KeyCode keycode() const noexcept;

	private:
		Win32KeyCode m_KeyCode;
		std::chrono::nanoseconds m_Start;
	};
}