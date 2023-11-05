#pragma once

#include "Types.hpp"
#include "../SupportedImageLayoutTransition.hpp"
#pragma warning(push, 0)
#include <vulkan/vulkan.hpp>
#include <array>
#include <tuple>
#include <algorithm>
#pragma warning(pop)

namespace API_NAME {
	std::array<SupportedImageLayoutTransition, 3> inline constexpr SUPPORTED_IMAGE_LAYOUT_TRANSITIONS{ {
		{	//Undefined to Dst
			.old_layout = VK_IMAGE_LAYOUT_UNDEFINED,
			.new_layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			.src_access_mask = 0,
			.dst_access_mask = VK_ACCESS_TRANSFER_WRITE_BIT,
			.src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			.dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT,
			.aspect = VK_IMAGE_ASPECT_COLOR_BIT
		}, {//Undefined to Depth Scentil
			.old_layout = VK_IMAGE_LAYOUT_UNDEFINED,
			.new_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			.src_access_mask = 0,
			.dst_access_mask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			.src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			.dst_stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
			.aspect = VK_IMAGE_ASPECT_DEPTH_BIT
		}, {//Dst to Shader Accessible
			.old_layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			.new_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			.src_access_mask = VK_ACCESS_TRANSFER_WRITE_BIT,
			.dst_access_mask = VK_ACCESS_SHADER_READ_BIT,
			.src_stage = VK_PIPELINE_STAGE_TRANSFER_BIT,
			.dst_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			.aspect = VK_IMAGE_ASPECT_COLOR_BIT
		} } };
	[[nodiscard]] constexpr SupportedImageLayoutTransition getSupportedImageLayoutTransition(VkImageLayout const old_layout, VkImageLayout const new_layout) noexcept {
		auto it = std::find_if(std::cbegin(SUPPORTED_IMAGE_LAYOUT_TRANSITIONS), std::cend(SUPPORTED_IMAGE_LAYOUT_TRANSITIONS), [&old_layout, &new_layout](SupportedImageLayoutTransition const& supported_image_layout_transition) { return supported_image_layout_transition.old_layout == old_layout && supported_image_layout_transition.new_layout == new_layout; });
		if (it != std::cend(SUPPORTED_IMAGE_LAYOUT_TRANSITIONS)) return *it;
		return {
			.old_layout = old_layout,
			.new_layout = new_layout,
			.src_access_mask = VK_ACCESS_FLAG_BITS_MAX_ENUM,
			.dst_access_mask = VK_ACCESS_FLAG_BITS_MAX_ENUM,
			.src_stage = VK_PIPELINE_STAGE_FLAG_BITS_MAX_ENUM,
			.dst_stage = VK_PIPELINE_STAGE_FLAG_BITS_MAX_ENUM
		};
	}

	[[nodiscard]] u32 findMemoryTypeIndex(VkPhysicalDevice const physical_device, u32 const memory_type_bits, VkMemoryPropertyFlags const memory_properties);
	[[nodiscard]] std::tuple<VkResult, VkBuffer, VkDeviceMemory> createBuffer(VkDevice const logical_device, VkPhysicalDevice const physical_device, VkDeviceSize const size, VkBufferUsageFlags const usage, VkMemoryPropertyFlags const memory_properties);
	[[nodiscard]] std::tuple<VkResult, VkImage, VkDeviceMemory> createImage(VkDevice const logical_device, VkPhysicalDevice const physical_device, u32 const width, u32 const height, VkFormat const format, VkImageTiling const tiling, VkImageUsageFlags const usage, VkMemoryPropertyFlags const memory_properties, u32 const layers = 1);
	[[nodiscard]] std::tuple<VkResult, VkImageView> createImageView(VkDevice const logical_device, VkImage const image, VkFormat const format, VkImageAspectFlags const aspect, VkImageViewType const type = VK_IMAGE_VIEW_TYPE_2D, u32 const layers = 1);
	VkResult copyBufferToImage(VkDevice const logical_device, VkCommandPool const command_pool, VkQueue const graphics_queue, VkBuffer const buffer, VkImage const image, u32 const width, u32 const height, u32 const layers = 1);
	[[nodiscard]] std::tuple<VkResult, VkBuffer, VkDeviceMemory> copyBuffer(VkDevice const logical_device, VkPhysicalDevice const physical_device, VkCommandBuffer const command_buffer, VkQueue const graphics_queue, VkBuffer const src);
	[[nodiscard]] std::tuple<VkResult, VkCommandBuffer> recordOnceOffCommand(VkDevice const logical_device, VkCommandPool const command_pool);
	VkResult destroyOnceOffCommand(VkDevice const logical_device, VkCommandPool const command_pool, VkCommandBuffer const command_buffer, VkQueue const graphics_queue);
	VkResult flushCommandBuffer(VkDevice const logical_device, VkCommandPool const command_pool, VkCommandBuffer const command_buffer, VkQueue const queue);
	VkResult transitionImageLayout(VkDevice const logical_device, VkCommandPool const command_pool, VkQueue const graphics_queue, VkImage const image, VkFormat const format, VkImageLayout const old_layout, VkImageLayout const new_layout, u32 const layers = 1);

	template <auto val> auto ConstantPredicate = [](auto&&...) -> decltype(val) { return val; };
}