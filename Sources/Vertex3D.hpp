#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#pragma warning(push, 0)
#include <spdlog/fmt/fmt.h>
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>
#include <vulkan/vulkan.hpp>
#include <array>
#include <xhash>
#pragma warning(pop)
	
namespace API_NAME {
	struct Vertex3D {
		glm::vec3 XYZ;
		glm::vec3 colour;
		glm::vec2 UV;

		std::string toString() const noexcept {
			return std::format("{},{},{}-{}{}{}-{}{}", XYZ.x, XYZ.y, XYZ.z, colour.r, colour.g, colour.b, UV.x, UV.y);
		}

		constexpr bool operator==(Vertex3D const& vertex) const noexcept {
			return XYZ == vertex.XYZ && colour == vertex.colour && UV == vertex.UV;
		}
		constexpr bool operator<(Vertex3D const& vertex) const noexcept {
			return std::tie(XYZ.x, XYZ.y, XYZ.z, colour.r, colour.g, colour.b, UV.x, UV.y) < std::tie(vertex.XYZ.x, vertex.XYZ.y, vertex.XYZ.z, vertex.colour.r, vertex.colour.g, vertex.colour.b, vertex.UV.x, vertex.UV.y);
		}

		VkVertexInputBindingDescription static constexpr BINDING_DESCRIPTION{
			.binding = 0,
			.stride = sizeof(glm::vec3) + sizeof(glm::vec3) + sizeof(glm::vec2),
			.inputRate = VK_VERTEX_INPUT_RATE_VERTEX
		};
		std::array<VkVertexInputAttributeDescription, 3> static constexpr ATTRIBUTE_DESCRIPTION { {
			/* XYZ */
			{
				.location = 0,
				.binding = 0,
				.format = VK_FORMAT_R32G32B32_SFLOAT,
				.offset = 0
			},
			/* colour */
			{
				.location = 1,
				.binding = 0,
				.format = VK_FORMAT_R32G32B32_SFLOAT,
				.offset = sizeof(glm::vec3)
			},
			/* UV */
			{
				.location = 2,
				.binding = 0,
				.format = VK_FORMAT_R32G32_SFLOAT,
				.offset = sizeof(glm::vec3) + sizeof(glm::vec3)
			}
		} };
	};
}

template <> struct std::hash<API_NAME::Vertex3D> {
	std::size_t operator()(API_NAME::Vertex3D const& vertex) const noexcept {
		return ((std::hash<glm::vec3>{}(vertex.XYZ) ^ (std::hash<glm::vec3>{}(vertex.colour) << 1)) >> 1) ^ (std::hash<glm::vec2>{}(vertex.UV) << 1); /* Pulled randomly off internet. Revise */
	}
};