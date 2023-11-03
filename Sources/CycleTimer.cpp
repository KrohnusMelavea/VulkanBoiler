#include "CycleTimer.hpp"

namespace API_NAME {
	CycleTimer::CycleTimer() : m_StartTime{ std::chrono::steady_clock::now() }, m_CurrentTime{ std::chrono::steady_clock::now() } {
		m_CycleTime = std::chrono::duration_cast<std::chrono::nanoseconds>(m_CurrentTime - m_StartTime);
	}

	std::chrono::nanoseconds CycleTimer::update() { /* reconsider */
		auto const current_time = std::chrono::steady_clock::now();;
		m_CycleTime = current_time - m_CurrentTime;
		m_CurrentTime = current_time;
		return m_CurrentTime - m_StartTime;
	}

	std::chrono::nanoseconds CycleTimer::getStartTime() {
		return m_StartTime.time_since_epoch();
	}
	std::chrono::nanoseconds CycleTimer::getCurrentTime() {
		return m_CurrentTime.time_since_epoch();
	}
	std::chrono::nanoseconds CycleTimer::getCycleTime() {
		return m_CycleTime;
	}
}