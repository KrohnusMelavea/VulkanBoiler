#include "Common/Util.hpp"
#include "Common/EnumStrings.hpp"
#include "VertexBufferObject.hpp"

#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#pragma warning(pop)

namespace API_NAME {
	VkResult VertexBufferObject::create(VkDevice const logical_device, VkPhysicalDevice const physical_device, std::span<Vertex3D> const& data) {
		VkResult result = VK_SUCCESS;

		result = m_VertexBuffer.create(logical_device, physical_device, std::size(data), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
#ifdef _DEBUG
		if (result != VK_SUCCESS) {
			SPDLOG_ERROR("Vertex Buffer failed creation: {}", getEnumString(result));
			return result;
		}
#endif
		(void)std::memcpy(m_VertexBuffer.mapped(), std::data(data), sizeof(Vertex3D) * std::size(data));

		return result;
	}
	void VertexBufferObject::free(VkDevice const logical_device) {
		m_VertexBuffer.free(logical_device);
	}

	MappedBuffer& VertexBufferObject::mapped_buffer() noexcept {
		return m_VertexBuffer;
	}
}