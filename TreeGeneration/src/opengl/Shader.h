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
	Shader& setUniform(const std::string& name, const T& value) {
		int32 loc = getUniformLocation(name);
		if (loc != -1) {
			setUniform<T>(static_cast<uint32> (loc), value);
		}
		return *this;
	}
	template<typename T>
	Shader& setUniform(const uint32 location, const T& value);
	int32 getUniformLocation(const std::string& name);
private:
	ShaderHandle handle;
	const std::string vertexPath, fragmentPath;
	std::unordered_map<std::string, uint32> uniformLocations;

	void compile();
};
