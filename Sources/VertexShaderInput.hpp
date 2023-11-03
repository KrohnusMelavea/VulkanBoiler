#pragma once

#include "Common/Types.hpp"
#include "Vertex2D.hpp"
#pragma warning(push, 0)
#include <glm/glm.hpp>
#include <span>
#include <vector>
#pragma warning(pop)

namespace API_NAME {
	/* Beware padding */
	class VertexShaderInput {
	public:
		VertexShaderInput(std::span<Vertex2D> const& vertices) noexcept;

		void* getVertexData() const noexcept;

	private:

	};
}