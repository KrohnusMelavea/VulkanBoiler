#include "Common/Util.hpp"
#include "Common/EnumStrings.hpp"
#include "UnmappedBuffer.hpp"

#pragma warning(push, 0)
#include <glm/glm.hpp>
#include <spdlog/spdlog.h>
#include <tuple>
#pragma warning(pop)

namespace API_NAME {
	VkResult UnmappedBuffer::create(VkDevice const logical_device, VkPhysicalDevice const physical_device, std::size_t const size, VkBufferUsageFlags const usage, VkMemoryPropertyFlags const memory_flags) {
		VkResult result = VK_SUCCESS;

		std::tie(result, m_Buffer, m_Memory) = createBuffer(logical_device, physical_device, size, usage, memory_flags);
#ifdef _DEBUG
		if (result != VK_SUCCESS) {
			SPDLOG_ERROR("createBuffer failed buffer creation: {}", getEnumString(result));
			return result;
		}
#endif

		return result;
	}
	void UnmappedBuffer::free(VkDevice const logical_device) {
		vkDestroyBuffer(logical_device, m_Buffer, nullptr);
		vkFreeMemory(logical_device, m_Memory, nullptr);
	}

	VkBuffer UnmappedBuffer::buffer() const noexcept {
		return m_Buffer;
	}
	VkDeviceMemory UnmappedBuffer::memory() const noexcept {
		return m_Memory;
	}
}