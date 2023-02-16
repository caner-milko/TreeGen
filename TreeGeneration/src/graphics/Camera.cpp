#include "./Camera.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
namespace tgen::graphics {
vec3 Camera::getCameraDirection() const {
	if (dir)
		return glm::normalize(*dir);

	return { glm::cos(glm::radians(yaw)) * glm::cos(glm::radians(pitch)),
			glm::sin(glm::radians(pitch)),
			glm::sin(glm::radians(yaw)) * glm::cos(glm::radians(pitch)) };
}

vec3 Camera::getCameraRight() const {
	return glm::normalize(glm::cross(getCameraDirection(), getCameraUp()));
}

vec3 Camera::getCameraUp() const {
	return  glm::vec3(0.0f, 1.0f, 0.0f);
}

mat4 Camera::getViewMatrix() const {
	return glm::lookAt(cameraPosition, cameraPosition + getCameraDirection(), getCameraUp());
}

mat4 Camera::getProjectionMatrix() const {
	if (std::holds_alternative<vec4>(projection)) {
		vec4 ortho = std::get<vec4>(projection);
		mat4 lightProjection = glm::ortho(ortho.x, ortho.y, ortho.z, ortho.w,
			nearPlane, farPlane);
		return lightProjection;
	}
	else {
		return glm::perspective(glm::radians(fov), std::get<float>(projection), nearPlane, farPlane);
	}
}

mat4 Camera::getVP() const {
	return getProjectionMatrix() * getViewMatrix();
}

float Camera::getPitch() const
{
	return pitch;
}

float Camera::getYaw() const
{
	return yaw;
}

float Camera::getFov() const
{
	return fov;
}


void Camera::setPitch(float pitch) {
	dir = std::nullopt;
	this->pitch = pitch;
	validateMaxMins();
}

void Camera::setYaw(float yaw) {
	dir = std::nullopt;
	this->yaw = yaw;
	validateMaxMins();
}
void Camera::setFov(float fov) {
	dir = std::nullopt;
	this->fov = fov;
	validateMaxMins();
}
void Camera::validateMaxMins() {
	pitch = glm::clamp(pitch, minPitch, maxPitch);

	fov = glm::clamp(fov, minFov, maxFov);

	float floored = glm::mod(yaw, 360.0f);
	yaw = floored;
}
}