#include "Util.hpp"
#include "EnumStrings.hpp"
#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#include <limits>
#pragma warning(pop)

namespace API_NAME {
	u32 findMemoryTypeIndex(VkPhysicalDevice const physical_device, u32 const memory_type_bits, VkMemoryPropertyFlags const memory_properties) {
		VkPhysicalDeviceMemoryProperties physical_device_memory_properties;
		vkGetPhysicalDeviceMemoryProperties(physical_device, &physical_device_memory_properties);
		for (u32 i = 0; i < physical_device_memory_properties.memoryTypeCount; ++i) {
			if ((memory_type_bits & (1 << i)) && (physical_device_memory_properties.memoryTypes[i].propertyFlags & memory_properties) == memory_properties) {
				return i;
			}
		}
		return std::numeric_limits<u32>::max();
	}
	std::tuple<VkResult, VkBuffer, VkDeviceMemory> createBuffer(VkDevice const logical_device, VkPhysicalDevice const physical_device, VkDeviceSize const size, VkBufferUsageFlags const usage, VkMemoryPropertyFlags const memory_properties) {
		VkResult result;
		VkBuffer buffer;
		VkDeviceMemory buffer_memory;
		/* Buffer Creation */ {
			VkBufferCreateInfo const create_info{
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.pNext{},
			.flags{},
			.size = size,
			.usage = usage,
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.queueFamilyIndexCount{},
			.pQueueFamilyIndices{}
			};
			result = vkCreateBuffer(logical_device, &create_info, nullptr, &buffer);
#ifdef _DEBUG
			if (result != VK_SUCCESS) {
				SPDLOG_ERROR("vkCreateBuffer failed buffer creation: {}", getEnumString(result));
				return { result, VK_NULL_HANDLE, VK_NULL_HANDLE };
			}
#endif
		}
		/* Buffer Allocation */ {
			VkMemoryRequirements memory_requirements;
			vkGetBufferMemoryRequirements(logical_device, buffer, &memory_requirements);
			VkMemoryAllocateInfo const memory_allocation_info{
				.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
				.pNext{},
				.allocationSize = memory_requirements.size,
				.memoryTypeIndex = findMemoryTypeIndex(physical_device, memory_requirements.memoryTypeBits, memory_properties)
			};
			result = vkAllocateMemory(logical_device, &memory_allocation_info, nullptr, &buffer_memory);
#ifdef _DEBUG
			if (result != VK_SUCCESS) {
				SPDLOG_ERROR("vkAllocateMemory failed buffer creation: {}", getEnumString(result));
				return { result, VK_NULL_HANDLE, VK_NULL_HANDLE };
			}
#endif
		}

		result = vkBindBufferMemory(logical_device, buffer, buffer_memory, 0);
#ifdef _DEBUG
		if (result != VK_SUCCESS) {
			SPDLOG_ERROR("vkBindBufferMemory failed buffer binding: {}", getEnumString(result));
			return { result, VK_NULL_HANDLE, VK_NULL_HANDLE };
		}
#endif

		return { result, buffer, buffer_memory };
	}
	std::tuple<VkResult, VkImage, VkDeviceMemory> createImage(VkDevice const logical_device, VkPhysicalDevice const physical_device, u32 const width, u32 const height, VkFormat const format, VkImageTiling const tiling, VkImageUsageFlags const usage, VkMemoryPropertyFlags const memory_properties, u32 const layers) {
		VkResult result;
		VkImage image;
		VkDeviceMemory image_memory;

		/* Image Creation */ {
			VkImageCreateInfo const create_info{
				.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
				.pNext{},
				.flags{},
				.imageType = VK_IMAGE_TYPE_2D,
				.format = format,
				.extent = {
					.width = width,
					.height = height,
					.depth = 1
				},
				.mipLevels = 1,
				.arrayLayers = layers,
				.samples = VK_SAMPLE_COUNT_1_BIT,
				.tiling = tiling,
				.usage = usage,
				.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
				.queueFamilyIndexCount{},
				.pQueueFamilyIndices{},
				.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
			};
			result = vkCreateImage(logical_device, &create_info, nullptr, &image);
#ifdef _DEBUG
			if (result != VK_SUCCESS) {
				SPDLOG_ERROR("vkCreateImage failed image creation: {}", getEnumString(result));
				return { result, VK_NULL_HANDLE, VK_NULL_HANDLE };
			}
#endif
		}

		/* Image Allocation */ {
			VkMemoryRequirements memory_requirements;
			vkGetImageMemoryRequirements(logical_device, image, &memory_requirements);
			VkMemoryAllocateInfo const memory_allocation_info{
				.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
				.pNext = nullptr,
				.allocationSize = memory_requirements.size,
				.memoryTypeIndex = findMemoryTypeIndex(physical_device, memory_requirements.memoryTypeBits, memory_properties)
			};
			result = vkAllocateMemory(logical_device, &memory_allocation_info, nullptr, &image_memory);
#ifdef _DEBUG
			if (result != VK_SUCCESS) {
				SPDLOG_ERROR("vkAllocateMemory failed memory allocation: {}", getEnumString(result));
				return { result, VK_NULL_HANDLE, VK_NULL_HANDLE };
			}
#endif
		}
		
		result = vkBindImageMemory(logical_device, image, image_memory, 0);
#ifdef _DEBUG
		if (result != VK_SUCCESS) {
			SPDLOG_ERROR("vkBindImageMemory failed image memory binding: {}", getEnumString(result));
			return { result, VK_NULL_HANDLE, VK_NULL_HANDLE };
		}
#endif

		return { result, image, image_memory };
	}
	std::tuple<VkResult, VkImageView> createImageView(VkDevice const logical_device, VkImage const image, VkFormat const format, VkImageAspectFlags const aspect, VkImageViewType const type, u32 const layers) {
		VkResult result;
		VkImageView image_view;

		/* Image View Creation */ {
			VkImageViewCreateInfo const image_view_create_info{
				.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
				.pNext{},
				.flags{},
				.image = image,
				.viewType = type,
				.format = format,
				.components = {
					.r = VK_COMPONENT_SWIZZLE_IDENTITY,
					.g = VK_COMPONENT_SWIZZLE_IDENTITY,
					.b = VK_COMPONENT_SWIZZLE_IDENTITY,
					.a = VK_COMPONENT_SWIZZLE_IDENTITY
				},
				.subresourceRange = {
					.aspectMask = aspect,
					.baseMipLevel = 0,
					.levelCount = 1,
					.baseArrayLayer = 0,
					.layerCount = layers
				}
			};

			result = vkCreateImageView(logical_device, &image_view_create_info, nullptr, &image_view);
#ifdef _DEBUG
			if (result != VK_SUCCESS) {
				SPDLOG_ERROR("vkCreateImageView failed image view creation: {}", getEnumString(result));
				return { result, VK_NULL_HANDLE };
			}
#endif
		}

		return { result, image_view };
	}

	VkResult copyBufferToImage(VkDevice const logical_device, VkCommandPool const command_pool, VkQueue const graphics_queue, VkBuffer const buffer, VkImage const image, u32 const width, u32 const height, u32 const layers) {
		auto [result, command_buffer] = recordOnceOffCommand(logical_device, command_pool);
#ifdef _DEBUG
		if (result != VK_SUCCESS) {
			SPDLOG_ERROR("recordOnceOffCommand failed to record once-off command: {}", getEnumString(result));
			return result;
		}
#endif

		/* Buffer To Image Copy */ {
			VkBufferImageCopy const buffer_image_copy{
				.bufferOffset = 0,
				.bufferRowLength = 0,
				.bufferImageHeight = 0,
				.imageSubresource = {
					.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
					.mipLevel = 0,
					.baseArrayLayer = 0,
					.layerCount = layers
				},
				.imageOffset = {
					.x = 0,
					.y = 0,
					.z = 0
				},
				.imageExtent = {
					.width = width,
					.height = height,
					.depth = 1
				}
			};
			vkCmdCopyBufferToImage(command_buffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &buffer_image_copy);
		}
		
		result = destroyOnceOffCommand(logical_device, command_pool, command_buffer, graphics_queue);
#ifdef _DEBUG
		if (result != VK_SUCCESS) {
			SPDLOG_ERROR("destroyOnceOffCommand failed to destroy once-off command: {}", getEnumString(result));
			return result;
		}
#endif

		return result;
	}
	std::tuple<VkResult, VkBuffer, VkDeviceMemory> copyBuffer(VkDevice const logical_device, VkPhysicalDevice const physical_device, VkCommandPool const command_pool, VkQueue const graphics_queue, VkBuffer const src_buffer, VkDeviceSize const size, VkBufferUsageFlags const usage, VkMemoryPropertyFlags const memory_properties) {
		VkResult result = VK_SUCCESS;

		VkBuffer buffer = VK_NULL_HANDLE;
		VkDeviceMemory buffer_memory = VK_NULL_HANDLE;
		
		std::tie(result, buffer, buffer_memory) = createBuffer(logical_device, physical_device, size, usage, memory_properties);
#ifdef _DEBUG
		if (result != VK_SUCCESS) {
			SPDLOG_ERROR("createBuffer failed buffer creation: {}", getEnumString(result));
			return { result, buffer, buffer_memory };
		}
#endif

		VkCommandBufferAllocateInfo const command_buffer_allocate_info{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.pNext = nullptr,
			.commandPool = command_pool,
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = 1
		};		
		VkCommandBuffer staging_command_buffer = VK_NULL_HANDLE;
		result = vkAllocateCommandBuffers(logical_device, &command_buffer_allocate_info, &staging_command_buffer);
#ifdef _DEBUG
		if (result != VK_SUCCESS) {
			SPDLOG_ERROR("vkAllocateCommandBuffers failed staging command buffer allocation: {}", getEnumString(result));
			return { result, buffer, buffer_memory };
		}
#endif

		VkBufferCopy const copy_region{
			.srcOffset = 0,
			.dstOffset = 0,
			.size = size
		};
		vkCmdCopyBuffer(staging_command_buffer, src_buffer, buffer, 1, &copy_region);
		result = flushCommandBuffer(logical_device, command_pool, staging_command_buffer, graphics_queue);
#ifdef _DEBUG
		if (result != VK_SUCCESS) {
			SPDLOG_ERROR("flushCommandBuffer failed to flush staging command buffer: {}", getEnumString(result));
			return { result, buffer, buffer_memory };
		}
#endif

		return { result, buffer, buffer_memory };
	}
	
	std::tuple<VkResult, VkCommandBuffer> recordOnceOffCommand(VkDevice const logical_device, VkCommandPool const command_pool) {
		VkResult result;
		VkCommandBuffer command_buffer;

		/* Command Buffer Allocation */ {
			VkCommandBufferAllocateInfo const command_buffer_allocation_info{
				.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
				.pNext{},
				.commandPool = command_pool,
				.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
				.commandBufferCount = 1
			};
			result = vkAllocateCommandBuffers(logical_device, &command_buffer_allocation_info, &command_buffer);
#ifdef _DEBUG
			if (result != VK_SUCCESS) {
				SPDLOG_ERROR("vkAllocateCommandBuffers failed command buffer allocation: {}", getEnumString(result));
				return { result, VK_NULL_HANDLE };
			}
#endif
		}

		/* Command Buffer Start */ {
			VkCommandBufferBeginInfo const command_buffer_begin_info{
				.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
				.pNext{},
				.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
				.pInheritanceInfo{}
			};
			result = vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info);
#ifdef _DEBUG
			if (result != VK_SUCCESS) {
				SPDLOG_ERROR("vkBeginCommandBuffer failed execution: {}", getEnumString(result));
				return { result, VK_NULL_HANDLE };
			}
#endif
		}
		
		return { result, command_buffer };
	}
	VkResult destroyOnceOffCommand(VkDevice const logical_device, VkCommandPool const command_pool, VkCommandBuffer const command_buffer, VkQueue const graphics_queue) {
		VkResult result;

		result = vkEndCommandBuffer(command_buffer);
#ifdef _DEBUG
		if (result != VK_SUCCESS) {
			SPDLOG_ERROR("vkEndCommandBuffer failed execution: {}", getEnumString(result));
			return result;
		}
#endif
		/* Command Submission */ {
			VkSubmitInfo const submit_info{
				.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
				.pNext{},
				.waitSemaphoreCount{},
				.pWaitSemaphores{},
				.pWaitDstStageMask{},
				.commandBufferCount = 1,
				.pCommandBuffers = &command_buffer,
				.signalSemaphoreCount{},
				.pSignalSemaphores{}
			};
			result = vkQueueSubmit(graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
#ifdef _DEBUG
			if (result != VK_SUCCESS) {
				SPDLOG_ERROR("vkQueueSubmit failed queue submission: {}", getEnumString(result));
				return result;
			}
#endif
		}
		
		result = vkQueueWaitIdle(graphics_queue);
#ifdef _DEBUG
		if (result != VK_SUCCESS) {
			SPDLOG_ERROR("vkQueueWaitIdle failed waiting somehow: {}", getEnumString(result));
			return result;
		}
#endif
		vkFreeCommandBuffers(logical_device, command_pool, 1, &command_buffer);

		return result;
	}

	VkResult flushCommandBuffer(VkDevice const logical_device, VkCommandPool const command_pool, VkCommandBuffer const command_buffer, VkQueue const queue) {
		VkResult result = VK_SUCCESS;

		result = vkEndCommandBuffer(command_buffer);
#ifdef _DEBUG
		if (result != VK_SUCCESS) {
			SPDLOG_ERROR("vkEndCommandBuffer failed to end command buffer: {}", getEnumString(result));
			return result;
		}
#endif
		
		VkFence fence;
		/* Fence Creation */ {
			VkFenceCreateInfo const fence_create_info{
				.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
				.pNext = nullptr,
				.flags = VK_FENCE_CREATE_SIGNALED_BIT
			};
			result = vkCreateFence(logical_device, &fence_create_info, nullptr, &fence);
#ifdef _DEBUG
			if (result != VK_SUCCESS) {
				SPDLOG_ERROR("vkCreateFence failed fence creation: {}", getEnumString(result));
				return result;
			}
#endif
		}
		/* Command Submission */ {
			VkSubmitInfo const queue_submit_info{
				.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
				.pNext = nullptr,
				.waitSemaphoreCount = 0,
				.pWaitSemaphores = nullptr,
				.pWaitDstStageMask = nullptr,
				.commandBufferCount = 1,
				.pCommandBuffers = &command_buffer,
				.signalSemaphoreCount = 0,
				.pSignalSemaphores = nullptr
			};
			result = vkQueueSubmit(queue, 1, &queue_submit_info, fence);
#ifdef _DEBUG
			if (result != VK_SUCCESS) {
				SPDLOG_ERROR("vkQueueSubmit failed queue submission: {}", getEnumString(result));
				return result;
			}
#endif
		}

		result = vkWaitForFences(logical_device, 1, &fence, VK_TRUE, std::numeric_limits<u64>::max());
#ifdef _DEBUG
		if (result != VK_SUCCESS) {
			SPDLOG_ERROR("vkWaitForFences failed waiting for fences: {}", getEnumString(result));
			return result;
		}
#endif
		vkDestroyFence(logical_device, fence, nullptr);

		return result;
	}
	
	VkResult transitionImageLayout(VkDevice const logical_device, VkCommandPool const command_pool, VkQueue const graphics_queue, VkImage const image, VkFormat const format, VkImageLayout const old_layout, VkImageLayout const new_layout, u32 const layers) {
		auto [result, command_buffer] = recordOnceOffCommand(logical_device, command_pool);
#ifdef _DEBUG
		if (result != VK_SUCCESS) {
			SPDLOG_ERROR("recordOnceOffCommand failed to record once-off command: {}", getEnumString(result));
			return result;
		}
#endif
		
		/* Barrier Creation */ {
			auto supported_image_layout_transition = getSupportedImageLayoutTransition(old_layout, new_layout);
			if (supported_image_layout_transition.src_access_mask == VK_ACCESS_FLAG_BITS_MAX_ENUM || supported_image_layout_transition.dst_access_mask == VK_ACCESS_FLAG_BITS_MAX_ENUM || supported_image_layout_transition.src_stage == VK_PIPELINE_STAGE_FLAG_BITS_MAX_ENUM || supported_image_layout_transition.dst_stage == VK_PIPELINE_STAGE_FLAG_BITS_MAX_ENUM || supported_image_layout_transition.aspect == VK_IMAGE_ASPECT_FLAG_BITS_MAX_ENUM) {
				SPDLOG_ERROR("Invalid Image Format Transition Arguments: old_layout = {}, new_layout = {}", getEnumString(old_layout), getEnumString(new_layout));
			}
			VkImageMemoryBarrier const image_memory_barriers{
				.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
				.pNext = nullptr,
				.srcAccessMask = supported_image_layout_transition.src_access_mask,
				.dstAccessMask = supported_image_layout_transition.dst_access_mask,
				.oldLayout = supported_image_layout_transition.old_layout,
				.newLayout = supported_image_layout_transition.new_layout,
				.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
				.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
				.image = image,
				.subresourceRange = {
					.aspectMask = (format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT) ? supported_image_layout_transition.aspect | VK_IMAGE_ASPECT_STENCIL_BIT : supported_image_layout_transition.aspect,
					.baseMipLevel = 0,
					.levelCount = 1,
					.baseArrayLayer = 0,
					.layerCount = layers
				}
			};
			vkCmdPipelineBarrier(command_buffer, supported_image_layout_transition.src_stage, supported_image_layout_transition.dst_stage, 0, 0, nullptr, 0, nullptr, 1, &image_memory_barriers);
		}

		result = destroyOnceOffCommand(logical_device, command_pool, command_buffer, graphics_queue);
#ifdef _DEBUG
		if (result != VK_SUCCESS) {
			SPDLOG_ERROR("destroyOnceOffCommand failed to descroy once-off command: {}", getEnumString(result));
			return result;
		}
#endif

		return result;
	}
}