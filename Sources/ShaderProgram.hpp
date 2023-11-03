#pragma once

#include "Common/Types.hpp"
#pragma warning(push, 0)
#include <span>
#include <vector>
#include <string_view>
#pragma warning(pop)

namespace API_NAME {
	class ShaderProgram {
	public:
		ShaderProgram(std::string_view const& source_path);

		std::span<u32> getShaderCode();

	private:
		std::vector<u32> m_ShaderCode;
	};
}