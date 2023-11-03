#pragma once

#define GLM_INCLUDE_EXPERIMENTAL

#pragma warning(push, 0)
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>
#include <vulkan/vulkan.hpp>
#include <array>
#include <xhash>
#pragma warning(pop)

namespace API_NAME {
	struct Vertex2D {
		glm::vec2 XY;
		glm::vec3 colour;
		glm::vec2 UV;

		constexpr bool operator==(Vertex2D const& vertex) const noexcept {
			return XY == vertex.XY && colour == vertex.colour && UV == vertex.UV;
		}

		VkVertexInputBindingDescription static constexpr BINDING_DESCRIPTION{
			.binding = 0,
			.stride = sizeof(glm::vec2) + sizeof(glm::vec3) + sizeof(glm::vec2),
			.inputRate = VK_VERTEX_INPUT_RATE_VERTEX
		};
		std::array<VkVertexInputAttributeDescription, 3> static constexpr ATTRIBUTE_DESCRIPTION { {
			/* XY */
			{
				.location = 0,
				.binding = 0,
				.format = VK_FORMAT_R32G32_SFLOAT,
				.offset = 0
			},
			/* colour */
			{
				.location = 1,
				.binding = 0,
				.format = VK_FORMAT_R32G32B32_SFLOAT,
				.offset = sizeof(glm::vec2)
			},
			/* UV */
			{
				.location = 2,
				.binding = 0,
				.format = VK_FORMAT_R32G32_SFLOAT,
				.offset = sizeof(glm::vec2) + sizeof(glm::vec3)
			}
		} };
	};
}