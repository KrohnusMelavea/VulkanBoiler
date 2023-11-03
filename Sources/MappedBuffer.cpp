#include "Common/Util.hpp"
#include "Common/EnumStrings.hpp"
#include "MappedBuffer.hpp"

#pragma warning(push, 0)
#include <glm/glm.hpp>
#include <spdlog/spdlog.h>
#include <tuple>
#pragma warning(pop)

namespace API_NAME {
	VkResult MappedBuffer::create(VkDevice const logical_device, VkPhysicalDevice const physical_device, std::size_t const size, VkBufferUsageFlags const usage, VkMemoryPropertyFlags const memory_flags) {
		VkResult result = VK_SUCCESS;

		std::tie(result, m_Buffer, m_Memory) = createBuffer(logical_device, physical_device, size, usage, memory_flags);
#ifdef _DEBUG
		if (result != VK_SUCCESS) {
			SPDLOG_ERROR("createBuffer failed buffer creation: {}", getEnumString(result));
			return result;
		}
#endif
		result = vkMapMemory(logical_device, m_Memory, 0, size, 0, &m_MappedMemory);
#ifdef _DEBUG
		if (result != VK_SUCCESS) {
			SPDLOG_ERROR("vkMapMemory failed memory mapping: {}", getEnumString(result));
			return result;
		}
#endif
		m_Size = size;

		return result;
	}
	void MappedBuffer::free(VkDevice const logical_device) {
		vkUnmapMemory(logical_device, m_Memory);
		vkDestroyBuffer(logical_device, m_Buffer, nullptr);
		vkFreeMemory(logical_device, m_Memory, nullptr);
	}

	VkBuffer MappedBuffer::buffer() const noexcept {
		return m_Buffer;
	}
	VkDeviceMemory MappedBuffer::memory() const noexcept {
		return m_Memory;
	}
	void* MappedBuffer::mapped() const noexcept {
		return m_MappedMemory;
	}
	std::size_t MappedBuffer::size() const noexcept {
		return m_Size;
	}
}