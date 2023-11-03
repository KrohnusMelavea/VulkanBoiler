#include "ShaderProgram.hpp"

#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#include <fstream>
#include <iostream>
#pragma warning(pop)


namespace API_NAME {
	ShaderProgram::ShaderProgram(std::string_view const& source_path) {
		std::ifstream compiled_file(source_path.data(), std::ios::binary);
#ifdef _DEBUG
		if (!compiled_file) {
			SPDLOG_ERROR("Failed to open shader program: \"{}\"", source_path);
		}
#endif
		compiled_file.seekg(0, std::ios::end);
		auto const size = compiled_file.tellg();
		compiled_file.seekg(0, std::ios::beg);
		m_ShaderCode.resize(size / sizeof(u32));
		compiled_file.read(reinterpret_cast<char*>(m_ShaderCode.data()), size);
		//m_ShaderCode = std::vector<u32>((std::istream_iterator<u32>(compiled_file)), std::istream_iterator<u32>());
	}

	std::span<u32> ShaderProgram::getShaderCode() {
		return m_ShaderCode;
	}
}