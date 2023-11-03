#pragma once

#include "Common/Types.hpp"
#include "WindowProperties.hpp"
#pragma warning(push, 0)
#include <vector>
#include <string>
#include <string_view>
#include <windows.h>
#pragma warning(pop)

namespace API_NAME {
	class WindowHandler {
	public:
		WindowHandler(WindowProperties const& properties = { DEFAULT_WIDTH, DEFAULT_HEIGHT, std::string(DEFAULT_TITLE) });
		~WindowHandler();
		WindowHandler(WindowHandler const&) = delete;
		WindowHandler(WindowHandler&&) = delete;
		void operator=(WindowHandler const&) = delete;
		void operator=(WindowHandler&&) = delete;

		bool handleEvent([[maybe_unused]] u32 message, [[maybe_unused]] u64 uparam, [[maybe_unused]] i64 iparam);
		void poll();

		[[nodiscard]] bool isOpen();

		[[nodiscard]] u32 getWidth();
		[[nodiscard]] u32 getHeight();
		[[nodiscard]] std::string_view getTitle();
		[[nodiscard]] WindowProperties getProperties(); /* String Copy */
		[[nodiscard]] HWND getHandle(); /* Careful */

		static std::vector<WindowHandler*> window_handlers;

	private:
		WNDCLASSW m_WindowClass;

		bool m_IsOpen;
		WindowProperties m_Properties;
		HWND m_WindowHandle;

		static constexpr u32 DEFAULT_WIDTH = 1920, DEFAULT_HEIGHT = 1000;
		static constexpr std::string_view DEFAULT_TITLE = "Default Window";
	};
}