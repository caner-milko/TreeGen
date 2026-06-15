#pragma once
#include "Common.h"
#include "opengl/types/Texture.h"
namespace tgen::graphics {
struct DirectionalLight {
	static DirectionalLight GDirLight;
	void SetDir(vec3 dir) { lightCam->dir = dir; lightDir = dir; }
	std::optional<Camera> lightCam = std::nullopt;
	vec3 lightDir;
	vec3 ambientCol;
	vec3 lightColor;
	rc<gl::Texture> shadowMap;
	//left, right, bottom, top
};


}