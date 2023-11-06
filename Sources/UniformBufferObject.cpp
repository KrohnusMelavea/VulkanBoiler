#include "Common/EnumStrings.hpp"
#include "UniformBufferObject.hpp"

#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#include <tuple>
#pragma warning(pop)

namespace API_NAME {
	UniformBufferObject::UniformBufferObject(VkDevice const logical_device, VkPhysicalDevice const physical_device) noexcept : m_LogicalDevice{ logical_device }, m_PhysicalDevice{ physical_device }, m_MappedBuffer(logical_device, physical_device) {

	}

	VkResult UniformBufferObject::create() {
		if (m_MappedBuffer.buffer() != VK_NULL_HANDLE) {
			SPDLOG_WARN("UniformBufferObject attempted buffer creation without destructing previously created buffer.");
			free();
		}

		VkResult result = VK_SUCCESS;
		
		result = m_MappedBuffer.create(sizeof(m_Uniform), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
#ifdef _DEBUG
		if (result != VK_SUCCESS) {
			SPDLOG_ERROR("MappedBuffer creation failed mapped buffer creation: {}", getEnumString(result));
			return result;
		}
#endif

		return result;
	}
	void UniformBufferObject::free() {
		m_MappedBuffer.free();
	}

	UniformData& UniformBufferObject::uniform() noexcept {
		return m_Uniform;
	}
	MappedBuffer& UniformBufferObject::mapped_buffer() noexcept {
		return m_MappedBuffer;
	}
}