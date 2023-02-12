#pragma once
#include "Camera.h"
namespace tgen::graphics {
struct DrawView {
	const Camera& camera;
	const mat4 VP;
	DrawView(const Camera& camera) : camera(camera), VP(camera.getVP()) {}
};
struct alignas(16) CameraUniform {
	mat4 vp;
	vec4 pos_near;
	vec4 dir_far;
	vec4 ortho;
	vec2 aspectRatio_projection;
	static CameraUniform fromCameraAndVP(const Camera& cam, const mat4& vp) {
		return CameraUniform{
			.vp = vp,
			.pos_near = {cam.cameraPosition, cam.nearPlane},
			.dir_far = {cam.getCameraDirection(), cam.farPlane},
			.ortho = std::holds_alternative<vec4>(cam.projection) ? std::get<vec4>(cam.projection) : vec4(0),
			.aspectRatio_projection = vec2(std::holds_alternative<float>(cam.projection) ? std::get<float>(cam.projection) : 0, std::holds_alternative<float>(cam.projection))
		};
	}
};


}