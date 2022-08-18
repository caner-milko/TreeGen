#include "./Camera.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
Camera::Camera(float aspectRatio) : aspectRatio(aspectRatio)
{
}
vec3 Camera::getCameraDirection() const {
	return glm::vec3{ glm::cos(glm::radians(yaw)) * glm::cos(glm::radians(pitch)),
			glm::sin(glm::radians(pitch)),
			glm::sin(glm::radians(yaw)) * glm::cos(glm::radians(pitch)) };
}

vec3 Camera::getCameraRight() const {
	return glm::normalize(glm::cross(getCameraDirection(), getCameraUp()));
}

vec3 Camera::getCameraUp() const {
	return  glm::vec3(0.0f, 1.0f, 0.0f);
}

vec3 Camera::getCameraPosition() const {
	return cameraPosition;
}

mat4 Camera::getViewMatrix() const {
	return glm::lookAt(cameraPosition, cameraPosition + getCameraDirection(), getCameraUp());
}

mat4 Camera::getProjectionMatrix() const {
	return glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
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

float Camera::getAspectRatio() const
{
	return aspectRatio;
}

float Camera::getNearPlane() const
{
	return nearPlane;
}

float Camera::getFarPlane() const
{
	return farPlane;
}


void Camera::setPitch(float pitch) {
	this->pitch = pitch;
	validateMaxMins();
}

void Camera::setYaw(float yaw) {
	this->yaw = yaw;
	validateMaxMins();
}
void Camera::setFov(float fov) {
	this->fov = fov;
	validateMaxMins();
}
void Camera::setAspectRatio(float aspectRatio) {
	this->aspectRatio = aspectRatio;
}
void Camera::setNearPlane(float nearPlane) {
	this->nearPlane = nearPlane;
}
void Camera::setFarPlane(float farPlane) {
	this->farPlane = farPlane;
}

void Camera::setCameraPosition(const glm::vec3& cameraPosition) {
	this->cameraPosition = cameraPosition;
}
void Camera::validateMaxMins() {
	pitch = glm::clamp(pitch, minPitch, maxPitch);

	fov = glm::clamp(fov, minFov, maxFov);

	float floored = glm::mod(yaw, 360.0f);
}
