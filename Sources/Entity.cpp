#include "Entity.hpp"

namespace API_NAME {
	Entity::Entity(id_type const& id, Mesh::id_type const& mesh_id, LayeredTexture::id_type const& layered_texture_id, glm::vec3 const& translation, glm::vec3 const& rotation, glm::vec3 const& scale, u32 const texture_index) : m_ID{ id }, m_MeshID{ mesh_id }, m_LayeredTextureID{ layered_texture_id }, m_EntityInstanceData{ translation, rotation, scale, texture_index } {

	}

	void Entity::displace(glm::vec3 const& translation) noexcept {
		m_EntityInstanceData.translation += translation;
	}
	void Entity::rotate(glm::vec3 const& rotation) noexcept {
		m_EntityInstanceData.rotation += rotation;
	}
	void Entity::grow(glm::vec3 const& growth) noexcept {
		m_EntityInstanceData.scale *= growth;
	}
	void Entity::shrink(glm::vec3 const& shrinkage) noexcept {
		m_EntityInstanceData.scale /= shrinkage;
	}

	glm::vec3& Entity::translation() noexcept {
		return m_EntityInstanceData.translation;
	}
	glm::vec3& Entity::rotation() noexcept {
		return m_EntityInstanceData.rotation;
	}
	glm::vec3& Entity::scale() noexcept {
		return m_EntityInstanceData.scale;
	}
	u32& Entity::texture_index() noexcept {
		return m_EntityInstanceData.texture_index;
	}
	EntityInstanceData& Entity::instance_data() noexcept {
		return m_EntityInstanceData;
	}

	LayeredTexture::id_type const& Entity::layered_texture_id() const noexcept {
		return m_LayeredTextureID;
	}
}