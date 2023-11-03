#include "Camera.hpp"

#pragma warning(push, 0)
#include <glm/gtx/transform.hpp>
#include <limits>
#pragma warning(pop)

namespace API_NAME {
	Camera::Camera(glm::vec3 const& translation, glm::vec3 const& rotation) : m_Translation{ translation }, m_Rotation{ rotation } {

	}

	void Camera::setTranslation(glm::vec3 const& translation) noexcept {
		m_Translation = translation;
	}
	void Camera::setRotation(glm::vec3 const& rotation) noexcept {
		m_Rotation = rotation;
	}

	void Camera::displace(glm::vec3 const& translation) noexcept {
		m_Translation += translation;
	}
	void Camera::rotate(glm::vec3 const& rotation) noexcept {
		m_Rotation += rotation;
	}

	glm::vec3 Camera::translation() const noexcept {
		return m_Translation;
	}
	glm::vec3 Camera::rotation() const noexcept {
		return m_Rotation;
	}

	glm::mat4 Camera::transformation() const noexcept {
		return glm::translate(glm::mat4(1.0f), m_Translation);
	}
}