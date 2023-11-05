#pragma once

#pragma warning(push, 0)
#include <vulkan/vulkan.hpp>
#pragma warning(pop)

namespace API_NAME {
	struct SupportedImageLayoutTransition {
		VkImageLayout old_layout, new_layout;
		VkAccessFlags src_access_mask, dst_access_mask;
		VkPipelineStageFlags src_stage, dst_stage;
		VkImageAspectFlags aspect;
	};
}