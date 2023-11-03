#pragma once

#include "Common/Types.hpp"

#pragma warning(push, 0)
#include <glm/glm.hpp>
#pragma warning(pop)

namespace API_NAME {
	struct EntityInstanceData {
		glm::vec3 translation;
		glm::vec3 rotation;
		glm::vec3 scale;
		u32 texture_index;
	};
}