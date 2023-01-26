#pragma once
#include <string>
#include <string_view>
#include <unordered_map>
#include "../Definitions.h"
#include <glad/glad.h>
using ShaderHandle = uint32;
class Shader {
public:
	struct UniformInfo
	{
		GLint location = -1;
		GLsizei count;
	};
	Shader(std::string_view vertexSrc, std::string_view fragmentSrc);
	~Shader();
	void bind() const;
	DELETE_COPY_CONSTRUCTORS(Shader)

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
	void getAllUniformLocations();
	int8 getTextureIndex(const std::string& name);
	int8 getTextureIndex(const uint32 location);
private:
	ShaderHandle handle;
	std::unordered_map<std::string, UniformInfo> uniformLocations;
	std::unordered_map<uint32, uint8> textureIndices;
	void compile(std::string_view vertexSrc, std::string_view fragmentSrc);
};
