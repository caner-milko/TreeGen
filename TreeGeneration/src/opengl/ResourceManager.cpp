#include <fstream>
#include <iostream>


#include "ResourceManager.h"

std::string ResourceManager::readTextFile(std::string_view path) const
{
	try
	{
		std::ifstream ifs(path.data());
		return std::string((std::istreambuf_iterator<char>(ifs)),
			(std::istreambuf_iterator<char>()));
	}
	catch (std::ifstream::failure e)
	{
		std::cerr << "Resource Manager: Couldn't read text file at path: " << path << std::endl;
	}
	return "";
}
Shader* ResourceManager::loadShader(const std::string& name, std::string_view vertex_path, std::string_view fragment_path)
{
	auto found = m_Shaders.find(name);
	if (found != m_Shaders.end())
		return &found->second;
	return &m_Shaders.insert({ name, Shader(vertex_path, fragment_path) }).first->second;
}

void ResourceManager::ClearResources()
{
	m_Shaders.clear();
}