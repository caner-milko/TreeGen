#include "Shader.h"
#include <glad/glad.h>
#include "./ResourceManager.h"
#include <iostream>
#include <glm/gtc/type_ptr.hpp>
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
	int32_t loc = glGetUniformLocation(handle, name.c_str());
	if (loc == -1)
		std::cout << "Warning: uniform '" << name << "' doesn't exist!" << std::endl;
	uniformLocations[name] = loc;
	return loc;
}


template<>
Shader& Shader::setUniform(const uint32 location, const bool& value)
{
	bind();
	glUniform1i(location, (int)value);
	return *this;
}template<>
Shader& Shader::setUniform(const uint32 location, const int& value)
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
		std::cout << "Shader Compilation: Error at Vertex Shader Compilation in path: " << fragmentPath << "\n" << infoLog << std::endl;
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
