#include "Application.hpp"
#include "WindowHandler.hpp"
#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#pragma warning(pop)

#include <iostream>
namespace API_NAME {
	std::vector<WindowHandler*> WindowHandler::window_handlers = {};

	i64 __stdcall forwardEvent([[maybe_unused]] HWND window_handle, [[maybe_unused]] u32 message, [[maybe_unused]] u64 uparam, [[maybe_unused]] i64 iparam) {
		auto location = std::find_if(std::begin(WindowHandler::window_handlers), std::end(WindowHandler::window_handlers), [&window_handle](WindowHandler* window_handler) { return window_handler->getHandle() == window_handle; });
		if (location == std::end(WindowHandler::window_handlers)) {
			return DefWindowProcW(window_handle, message, uparam, iparam);
		}
		if (!(*location)->handleEvent(message, uparam, iparam)) {
			return DefWindowProcW(window_handle, message, uparam, iparam);
		}
		/* Magic */
		return reinterpret_cast<Application*>(GetWindowLongPtrW(window_handle, 0))->handleEvent(window_handle, message, uparam, iparam);
	}

	WindowHandler::WindowHandler(WindowProperties const& properties) : 
		m_WindowClass{
			.style{},
			.lpfnWndProc = forwardEvent,
			.cbClsExtra{},
			.cbWndExtra = sizeof(Application*),
			.hInstance = GetModuleHandleW(NULL),
			.hIcon{},
			.hCursor{},
			.hbrBackground{},
			.lpszMenuName{},
			.lpszClassName = L"test",
		},
		m_Properties{ properties }
	{
		window_handlers.push_back(this);

		RegisterClassW(&m_WindowClass);
		m_WindowHandle = CreateWindowExW(
			0, 
			L"test", std::wstring(m_Properties.title.begin(), m_Properties.title.end()).c_str(), 
			WS_OVERLAPPEDWINDOW, 
			CW_USEDEFAULT, CW_USEDEFAULT, static_cast<i32>(m_Properties.width), static_cast<i32>(m_Properties.height), 
			NULL, NULL, m_WindowClass.hInstance, NULL);
		if (m_WindowHandle == nullptr) {
			SPDLOG_ERROR("Window Failed Creation");
			m_IsOpen = false;
			return;
		}
		ShowWindow(m_WindowHandle, SW_NORMAL);
		m_IsOpen = true;
	}
	WindowHandler::~WindowHandler() {
		PostQuitMessage(NULL);
		DestroyWindow(m_WindowHandle);
		UnregisterClassW(m_WindowClass.lpszClassName, m_WindowClass.hInstance);
	}

	bool WindowHandler::handleEvent([[maybe_unused]] u32 message, [[maybe_unused]] u64 uparam, [[maybe_unused]] i64 iparam) {
		auto application = reinterpret_cast<Application*>(GetWindowLongPtrW(m_WindowHandle, 0));
		application->handleEvent(m_WindowHandle, message, uparam, iparam);

		switch (message) {
		case WM_DESTROY:
			m_IsOpen = false;
			return true;
		case WM_PAINT:
			return true;
		}
		return false;
	}
	void WindowHandler::poll() {
		MSG message{};
		if (GetMessageW(&message, m_WindowHandle, 0, 0) <= 0) {
			m_IsOpen = false;
			return;
		}
		TranslateMessage(&message);
		DispatchMessageW(&message);
	}

	bool WindowHandler::isOpen() {
		return m_IsOpen;
	}

	u32 WindowHandler::getWidth() {
		return m_Properties.width;
	}
	u32 WindowHandler::getHeight() {
		return m_Properties.height;
	}
	std::string_view WindowHandler::getTitle() {
		return m_Properties.title;
	}
	WindowProperties WindowHandler::getProperties() {
		return m_Properties;
	}
	HWND WindowHandler::getHandle() {
		return m_WindowHandle;
	}
}