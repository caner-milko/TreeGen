#pragma once
#include "Common.h"
#include "Light.h"
namespace tgen::graphics {
struct DrawScene {
	DrawScene(DirectionalLight const& light) : light(light), lightVP(light.lightCam->getVP()) {}
	DirectionalLight const& light;
	mat4 lightVP;
};
struct alignas(16) DirLightUniform {
	vec4 lightColor;
	vec4 ambientColor;
	CameraUniform cam;
	static DirLightUniform fromLightAndVP(const DirectionalLight& light, const mat4& vp) {
		return DirLightUniform{
			.lightColor = vec4(light.lightColor, 0),
			.ambientColor = vec4(light.ambientCol, 0),
			.cam = CameraUniform::fromCameraAndVP(*light.lightCam, vp) };
	}
};
}