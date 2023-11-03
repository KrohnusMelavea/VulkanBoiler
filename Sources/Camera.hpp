#pragma once

#include "Common/Types.hpp"

#pragma warning(push, 0)
#include <glm/glm.hpp>
#pragma warning(pop)

namespace API_NAME {
	class Camera {
	public:
		Camera() = default;
		Camera(glm::vec3 const& translation, glm::vec3 const& rotation);

		void setTranslation(glm::vec3 const& translation) noexcept;
		void setRotation(glm::vec3 const& rotation) noexcept;

		void displace(glm::vec3 const& translation) noexcept;
		void rotate(glm::vec3 const& rotation) noexcept;

		glm::vec3 translation() const noexcept;
		glm::vec3 rotation() const noexcept;

		glm::mat4 transformation() const noexcept;

	private:
		glm::vec3 m_Translation;
		glm::vec3 m_Rotation;
	};
}