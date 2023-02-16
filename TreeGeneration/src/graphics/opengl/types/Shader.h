#pragma once
#include <string>
#include <string_view>
#include <unordered_map>
#include "Common.h"
#include <glad/glad.h>
namespace tgen::graphics::gl {
using ShaderHandle = uint32;
class Shader {
public:
	struct UniformInfo
	{
		GLint location = -1;
		GLsizei count;
	};
	Shader() = default;
	bool init(std::string_view vertexSrc, std::string_view fragmentSrc, bool reinit = true);
	Shader(Shader&& other) noexcept
		: uniformLocations(std::move(other.uniformLocations)),
		textureIndices(std::move(other.textureIndices)),
		handle(std::exchange(handle, 0))
	{}
	~Shader() { destroy(); }
	ShaderHandle getHandle() const { return handle; }
	MOVE_OPERATOR(Shader);
	DELETE_COPY_CONSTRUCTORS(Shader)
		GL_OPERATOR_BOOL(handle);

	template<typename T>
	Shader& setUniform(std::string_view name, const T& value) {
		int32 loc = getUniformLocation(std::string(name));
		if (loc != -1) {
			setUniform<T>(static_cast<uint32> (loc), value);
		}
		return *this;
	}
	template<typename T>
	Shader& setUniform(const uint32 location, const T& value);
	int32 getUniformLocation(const std::string& name);
	UniformInfo getUniformInfo(const std::string& name);
	int8 getTextureIndex(const std::string& name);
	int8 getTextureIndex(const uint32 location);
private:
	void destroy() {
		glDeleteProgram(handle);
		handle = 0;
		uniformLocations = {};
		textureIndices = {};
	}
	ShaderHandle handle = 0;
	std::unordered_map<std::string, UniformInfo> uniformLocations{};
	std::unordered_map<uint32, uint8> textureIndices{};
	void compile(std::string_view vertexSrc, std::string_view fragmentSrc);
	void getAllUniformLocations();
};
}