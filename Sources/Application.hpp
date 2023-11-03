#pragma once

#include "Common/Types.hpp"
#include "KeyPress.hpp"
#include "Renderer.hpp"
#include "WindowHandler.hpp"
#pragma warning(push, 0)
#include <array>
#pragma warning(pop)

namespace API_NAME {
	enum class ExitStatus : u08 {
		AppDone,
		Failure,
		Restart,
	};

	class Application {
	public:
		Application();
		~Application();
		Application(Application const&) = delete;
		Application(Application&&) = delete;
		void operator=(Application const&) = delete;
		void operator=(Application&&) = delete;

		ExitStatus run();
		i64 handleEvent([[maybe_unused]] HWND window_handle, [[maybe_unused]] u32 message, [[maybe_unused]] u64 uparam, [[maybe_unused]] i64 iparam);

	private:
		KeyPress& GetKeyPress(Win32KeyCode const keycode) noexcept;

		std::array<KeyPress, std::size(WIN32_KEYCODE_ENUMS)> m_KeyPresses;
		WindowHandler m_WindowHandler;
		Renderer m_Renderer;
	};
}