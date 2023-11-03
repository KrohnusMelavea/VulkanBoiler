#pragma once

#include "Mesh.hpp"
#include "LayeredTexture.hpp"
#include "EntityInstanceData.hpp"

#pragma warning(push, 0)
#include <glm/glm.hpp>
#include <string_view>
#pragma warning(pop)

namespace API_NAME {
	class Entity {
	public:
		using id_type = std::string;
		std::string_view static constexpr NULL_ID = "";

		Entity() = default;
		Entity(id_type const& id, Mesh::id_type const& mesh_id, LayeredTexture::id_type const& layered_texture_id, glm::vec3 const& translation, glm::vec3 const& rotation, glm::vec3 const& scale, u32 const texture_index);

		void displace(glm::vec3 const& translation) noexcept;
		void rotate(glm::vec3 const& rotation) noexcept;
		void grow(glm::vec3 const& growth) noexcept;
		void shrink(glm::vec3 const& shrinkage) noexcept;

		glm::vec3& translation() noexcept;
		glm::vec3& rotation() noexcept;
		glm::vec3& scale() noexcept;
		u32& texture_index() noexcept;
		EntityInstanceData& instance_data() noexcept;
		LayeredTexture::id_type const& layered_texture_id() const noexcept;

	private:
		id_type m_ID;
		Mesh::id_type m_MeshID;
		LayeredTexture::id_type m_LayeredTextureID;
		EntityInstanceData m_EntityInstanceData;
	};
}