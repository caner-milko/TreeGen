#pragma once
#include "./Camera.h"
class Camera;
struct DrawView {
	const Camera& camera;
	DrawView(const Camera& camera) : camera(camera) {}
};