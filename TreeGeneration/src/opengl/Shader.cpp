#include "Shader.h"
#include <glad/glad.h>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>

#include <algorithm>


template<>
Shader& Shader::setUniform(const uint32 location, const bool& value)
{
	glProgramUniform1i(handle, location, (int)value);
	return *this;
}
template<>
Shader& Shader::setUniform(const uint32 location, const int32& value)
{
	glProgramUniform1i(handle, location, value);
	return *this;
}
template<>
Shader& Shader::setUniform(const uint32 location, const float& val)
{
	glProgramUniform1f(handle, location, val);
	return *this;
}
template<>
Shader& Shader::setUniform(const uint32 location, const glm::vec1& val)
{
	glProgramUniform1f(handle, location, val.x);
	return *this;
}
template<>
Shader& Shader::setUniform(const uint32 location, const vec2& val)
{
	glProgramUniform2f(handle, location, val.x, val.y);
	return *this;
}
template<>
Shader& Shader::setUniform(const uint32 location, const vec3& val)
{
	glProgramUniform3f(handle, location, val.x, val.y, val.z);
	return *this;
}
template<>
Shader& Shader::setUniform(const uint32 location, const vec4& val)
{
	glProgramUniform4f(handle, location, val.x, val.y, val.z, val.w);
	return *this;
}
template<>
Shader& Shader::setUniform(const uint32 location, const mat4& val)
{
	glProgramUniformMatrix4fv(handle, location, 1, GL_FALSE, glm::value_ptr(val));
	return *this;
}


Shader::Shader(std::string_view vertexSrc, std::string_view fragmentSrc)
{
	compile(vertexSrc, fragmentSrc);
	getAllUniformLocations();
}

Shader::~Shader()
{
	glDeleteProgram(handle);
}

void Shader::bind() const
{
	glUseProgram(handle);
}

int32 Shader::getUniformLocation(const std::string& name)
{
	return getUniformInfo(name).location;
}

Shader::UniformInfo Shader::getUniformInfo(const std::string& name)
{
	auto it = uniformLocations.find(name);
	if (it != uniformLocations.end())
		return it->second;
	return {};
}

void Shader::getAllUniformLocations()
{
	GLint uniform_count = 0;
	glGetProgramiv(handle, GL_ACTIVE_UNIFORMS, &uniform_count);

	if (uniform_count != 0)
	{
		GLint 	max_name_len = 0;
		GLsizei length = 0;
		GLsizei count = 0;
		GLenum 	type = GL_NONE;
		glGetProgramiv(handle, GL_ACTIVE_UNIFORM_MAX_LENGTH, &max_name_len);

		auto uniform_name = std::make_unique<char[]>(max_name_len);

		for (GLint i = 0; i < uniform_count; ++i)
		{
			glGetActiveUniform(handle, i, max_name_len, &length, &count, &type, uniform_name.get());

			UniformInfo ui = {};
			ui.location = glGetUniformLocation(handle, uniform_name.get());
			ui.count = count;

			uniformLocations.emplace(std::make_pair(std::string(uniform_name.get(), length), ui));
		}
	}
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
	uint8 index = textureIndices.insert({ location, static_cast<uint8>(textureIndices.size()) }).first->second;
	setUniform<int32>(location, index);
	return index;
}


void Shader::compile(std::string_view vertexSrc, std::string_view fragmentSrc)
{
	const char* vertexShaderCode = vertexSrc.data();
	const char* fragmentShaderCode = fragmentSrc.data();

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
		std::cout << "Shader Compilation: Error at Vertex Shader Compilation\n" << infoLog << std::endl;
		std::exit(1);
	}

	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fragmentShaderCode, NULL);
	glCompileShader(fragment);

	glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		glGetShaderInfoLog(fragment, 512, NULL, infoLog);
		std::cout << "Shader Compilation: Error at Fragment Shader Compilation\n" << infoLog << std::endl;
		std::exit(1);
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
