#include "Common/Util.hpp"
#include "Common/EnumStrings.hpp"
#include "UnmappedBuffer.hpp"

#pragma warning(push, 0)
#include <glm/glm.hpp>
#include <spdlog/spdlog.h>
#include <tuple>
#pragma warning(pop)

namespace API_NAME {
	UnmappedBuffer::UnmappedBuffer(VkBuffer const buffer, VkDeviceMemory const memory, VkDeviceSize const size) : m_Buffer{ buffer }, m_Memory{ memory }, m_Size{ size } {
		
	}

	void UnmappedBuffer::create(VkBuffer const buffer, VkDeviceMemory const memory, VkDeviceSize const size) {
#ifdef _DEBUG
		if (m_Buffer != VK_NULL_HANDLE) {
			SPDLOG_WARN("Forgot to free UnmappedBuffer prior to calling create().");
			//cannot free
		}
#endif
		m_Buffer = buffer;
		m_Memory = memory;
		m_Size = size;
	}
	VkResult UnmappedBuffer::create(VkDevice const logical_device, VkPhysicalDevice const physical_device, VkDeviceSize const size, VkBufferUsageFlags const usage, VkMemoryPropertyFlags const memory_flags) {
#ifdef _DEBUG
		if (m_Buffer != VK_NULL_HANDLE) /* Added in debug-only, because I've decided that buffers MUST be in a freed state prior to creation. */ {
			SPDLOG_WARN("Forgot to free UnmappedBuffer before calling create().");
			free(logical_device);
		}
#endif
		VkResult result = VK_SUCCESS;

		std::tie(result, m_Buffer, m_Memory) = createBuffer(logical_device, physical_device, size, usage, memory_flags);
#ifdef _DEBUG
		if (result != VK_SUCCESS) {
			SPDLOG_ERROR("createBuffer failed buffer creation: {}", getEnumString(result));
			return result;
		}
#endif
		m_Size = size;

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
	VkDeviceSize UnmappedBuffer::size() const noexcept {
		return m_Size;
	}
}