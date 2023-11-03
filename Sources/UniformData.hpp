#pragma once

#pragma warning(push, 0)
#include <glm/glm.hpp>
#pragma warning(pop)

namespace API_NAME {
	struct UniformData {
		alignas(64) glm::mat4 view;
		alignas(64) glm::mat4 projection;
	};
}