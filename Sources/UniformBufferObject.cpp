#include "Common/EnumStrings.hpp"
#include "UniformBufferObject.hpp"

#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#include <tuple>
#pragma warning(pop)

namespace API_NAME {
	VkResult UniformBufferObject::create(VkDevice const logical_device, VkPhysicalDevice const physical_device) {
		VkResult result = VK_SUCCESS;
		
		result = m_MappedBuffer.create(logical_device, physical_device, sizeof(m_Uniform), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
#ifdef _DEBUG
		if (result != VK_SUCCESS) {
			SPDLOG_ERROR("MappedBuffer::create failed mapepd buffer creation: {}", getEnumString(result));
			return result;
		}
#endif

		return result;
	}
	void UniformBufferObject::free(VkDevice const logical_device) {
		m_MappedBuffer.free(logical_device);
	}

	UniformData& UniformBufferObject::uniform() noexcept {
		return m_Uniform;
	}
	MappedBuffer& UniformBufferObject::mapped_buffer() noexcept {
		return m_MappedBuffer;
	}
}