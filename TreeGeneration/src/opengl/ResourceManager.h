#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <string_view>

#include "./Shader.h"
class ResourceManager {
public:
	static ResourceManager& getInstance() {
		static ResourceManager instance;
		return instance;
	}
	std::string readTextFile(std::string_view path) const;
	Shader* loadShader(const std::string& name, std::string_view vertex_path, std::string_view fragment_path);
	void ClearResources();
private:
	std::unordered_map < std::string, Shader> m_Shaders;
};