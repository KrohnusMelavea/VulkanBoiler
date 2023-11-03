#include "Common/EnumStrings.hpp"
#include "Application.hpp"

#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#include <algorithm>
#pragma warning(pop)

namespace API_NAME {
	Application::Application() : m_WindowHandler(WindowProperties{800, 600, "Vulkan Boiler"}), m_Renderer(m_WindowHandler.getHandle()) {
		(void)std::transform(std::cbegin(WIN32_KEYCODE_ENUMS), std::cend(WIN32_KEYCODE_ENUMS), std::begin(m_KeyPresses), [](auto&& entry) -> KeyPress { return { entry.first }; });
		(void)SetWindowLongPtrW(m_WindowHandler.getHandle(), 0, reinterpret_cast<i64>(this));
	}
	Application::~Application() {

	}

	ExitStatus Application::run() {
		while (m_WindowHandler.isOpen()) {
			if (GetKeyPress(Win32KeyCode::kw).isPressed()) {
				m_Renderer.camera().displace(glm::vec3(0.0f, -0.1f, 0.0f));
			}
			if (GetKeyPress(Win32KeyCode::ka).isPressed()) {
				m_Renderer.camera().displace(glm::vec3(-0.1f, 0.0f, 0.0f));
			}
			if (GetKeyPress(Win32KeyCode::ks).isPressed()) {
				m_Renderer.camera().displace(glm::vec3(0.0f, 0.1f, 0.0f));
			}
			if (GetKeyPress(Win32KeyCode::kd).isPressed()) {
				m_Renderer.camera().displace(glm::vec3(0.1f, 0.0f, 0.0f));
			}
			if (GetKeyPress(Win32KeyCode::space).isPressed()) {
				m_Renderer.camera().displace(glm::vec3(0.0f, 0.0f, 0.1f));
			}
			if (GetKeyPress(Win32KeyCode::shift).isPressed()) {
				m_Renderer.camera().displace(glm::vec3(0.0f, 0.0f, -0.1f));
			}
			m_WindowHandler.poll();
			m_Renderer.drawFrame();
		}
		m_Renderer.awaitClose();
		return ExitStatus::AppDone;
	}

	i64 Application::handleEvent([[maybe_unused]] HWND window_handle, [[maybe_unused]] u32 message, [[maybe_unused]] u64 uparam, [[maybe_unused]] i64 iparam) {
		switch (message) {
		case WM_KEYDOWN: {
			auto const keycode = static_cast<Win32KeyCode>(uparam);

			auto& keypress = GetKeyPress(keycode);
			keypress.press();

#pragma warning(push)
#pragma warning(disable : 4062)
			switch (keycode) {
			case Win32KeyCode::escape:
				m_Renderer.togglePaused();
				return 0;
			default:
				return 0;
			}
#pragma warning(pop)
			break;
		}
		case WM_KEYUP: {
			auto const keycode = static_cast<Win32KeyCode>(uparam);

			auto& keypress = GetKeyPress(keycode);
			keypress.release();

#pragma warning(push)
#pragma warning(disable : 4062)
			switch (keycode) {
			default:
				return 0;
			}
#pragma warning(pop)
			break;
		}
			
		case WM_MOUSEMOVE:
			return 0;
		case WM_SIZE:
			switch (uparam) {
			case SIZE_MINIMIZED:
			case SIZE_MAXIMIZED:
				m_Renderer.resized = true;
				break;
			}
			break;
		case WM_SIZING:
			m_Renderer.resized = true;
			break;
		}
		return 0;
	}

	KeyPress& Application::GetKeyPress(Win32KeyCode const keycode) noexcept {
		auto it = std::find_if(std::begin(m_KeyPresses), std::end(m_KeyPresses), [&keycode](auto&& keypress) { return keypress.keycode() == keycode; });
#ifdef _DEBUG
		if (it == std::end(m_KeyPresses)) {
			SPDLOG_ERROR("Invalid Key Code: {}", static_cast<u64>(keycode));
		}
#endif
		return *it;
	}
}