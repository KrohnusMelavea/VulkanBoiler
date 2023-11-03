#pragma once

#pragma warning(push, 0)
#include <vulkan/vulkan.hpp>
#pragma warning(pop)

namespace API_NAME {
	struct PipelineShaderStageCreateInfos {
		VkPipelineShaderStageCreateInfo vertex_stage_create_info, fragment_stage_create_info;
	};
}