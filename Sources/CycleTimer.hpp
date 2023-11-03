#pragma once

#pragma warning(push, 0)
#include <chrono>
#pragma warning(pop)

namespace API_NAME {
	class CycleTimer {
	public:
		CycleTimer();

		std::chrono::nanoseconds update();

		std::chrono::nanoseconds getStartTime();
		std::chrono::nanoseconds getCurrentTime();
		std::chrono::nanoseconds getCycleTime();

	private:
		std::chrono::steady_clock::time_point m_StartTime;					/* Time at object construction */
		std::chrono::steady_clock::time_point m_CurrentTime;				/* Time at most recent refresh */
		std::chrono::nanoseconds m_CycleTime;		/* Time difference between two most recent refreshes */
	};
}