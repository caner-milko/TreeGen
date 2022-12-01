#include "Shader.h"
#include <glad/glad.h>
#include "./ResourceManager.h"
#include <iostream>
#include <glm/gtc/type_ptr.hpp>

#include <algorithm>


template<>
Shader& Shader::setUniform(const uint32 location, const bool& value)
{
	bind();
	glUniform1i(location, (int)value);
	return *this;
}
template<>
Shader& Shader::setUniform(const uint32 location, const int32& value)
{
	bind();
	glUniform1i(location, value);
	return *this;
}
template<>
Shader& Shader::setUniform(const uint32 location, const float& val)
{
	bind();
	glUniform1f(location, val);
	return *this;
}
template<>
Shader& Shader::setUniform(const uint32 location, const glm::vec1& val)
{
	bind();
	glUniform1f(location, val.x);
	return *this;
}
template<>
Shader& Shader::setUniform(const uint32 location, const vec2& val)
{
	bind();
	glUniform2f(location, val.x, val.y);
	return *this;
}
template<>
Shader& Shader::setUniform(const uint32 location, const vec3& val)
{
	bind();
	glUniform3f(location, val.x, val.y, val.z);
	return *this;
}
template<>
Shader& Shader::setUniform(const uint32 location, const vec4& val)
{
	bind();
	glUniform4f(location, val.x, val.y, val.z, val.w);
	return *this;
}
template<>
Shader& Shader::setUniform(const uint32 location, const mat4& val)
{
	bind();
	glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(val));
	return *this;
}


Shader::Shader(std::string_view vertexPath, std::string_view fragmentPath) : vertexPath(vertexPath), fragmentPath(fragmentPath)
{
	compile();
}

void Shader::bind() const
{
	glUseProgram(handle);
}

void Shader::destroy()
{
	glDeleteProgram(handle);
}

int32 Shader::getUniformLocation(const std::string& name)
{
	auto found = uniformLocations.find(name);
	if (found != uniformLocations.end())
		return found->second;
	int32 loc = glGetUniformLocation(handle, name.c_str());
	if (loc == -1)
		std::cout << "Warning: uniform '" << name << "' doesn't exist!" << std::endl;
	uniformLocations[name] = loc;
	return loc;
}

int8 Shader::getTextureIndex(const std::string& name)
{
	int32 uniformLoc = getUniformLocation(name);
	if (uniformLoc == -1)
		return -1;
	return getTextureIndex(uniformLoc);
}

int8 Shader::getTextureIndex(const uint32 location)
{
	auto found = textureIndices.find(location);
	if (found != textureIndices.end())
		return found->second;
	uint8 index = textureIndices.insert({ location, textureIndices.size() }).first->second;
	bind();
	setUniform<int32>(location, index);
	return index;
}


void Shader::compile()
{
	std::string vertexShaderCodeStr = ResourceManager::getInstance().readTextFile(vertexPath);
	std::string fragmentShaderCodeStr = ResourceManager::getInstance().readTextFile(fragmentPath);

	const char* vertexShaderCode = vertexShaderCodeStr.c_str();
	const char* fragmentShaderCode = fragmentShaderCodeStr.c_str();

	uint32 vertex, fragment;
	int32 success;
	char infoLog[512];

	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vertexShaderCode, NULL);
	glCompileShader(vertex);

	glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		glGetShaderInfoLog(vertex, 512, NULL, infoLog);
		std::cout << "Shader Compilation: Error at Vertex Shader Compilation in path: " << vertexPath << "\n" << infoLog << std::endl;
	}

	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fragmentShaderCode, NULL);
	glCompileShader(fragment);

	glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		glGetShaderInfoLog(fragment, 512, NULL, infoLog);
		std::cout << "Shader Compilation: Error at Fragment Shader Compilation in path: " << fragmentPath << "\n" << infoLog << std::endl;
	}

	handle = glCreateProgram();
	glAttachShader(handle, vertex);
	glAttachShader(handle, fragment);
	glLinkProgram(handle);

	glGetProgramiv(handle, GL_LINK_STATUS, &success);

	if (!success)
	{
		glGetProgramInfoLog(handle, 512, NULL, infoLog);
		std::cout << "Shader Compilation: Linking failed.\n" << infoLog << std::endl;
	}

	glDeleteShader(vertex);
	glDeleteShader(fragment);
}
