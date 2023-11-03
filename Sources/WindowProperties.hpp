#pragma once

#include "Common/Types.hpp"
#pragma warning(push, 0)
#include <string>
#pragma warning(pop)

namespace API_NAME {
	struct WindowProperties {
		u32 width, height;
		std::string title;
	};
}