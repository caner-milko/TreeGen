#pragma once
#include <string>
#include <string_view>
#include <unordered_map>
#include "../Definitions.h"
using ShaderHandle = uint32;
class Shader {
public:
	Shader(std::string_view vertexPath, std::string_view fragmentPath);
	void bind() const;
	void destroy();
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
	int8 getTextureIndex(const uint32 location);
private:
	ShaderHandle handle;
	const std::string vertexPath, fragmentPath;
	std::unordered_map<std::string, uint32> uniformLocations;
	std::unordered_map<uint32, uint8> textureIndices;
	void compile();
};
