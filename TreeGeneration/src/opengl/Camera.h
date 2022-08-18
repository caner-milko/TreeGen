#pragma once
#include "../Definitions.h"
class Camera {
public:
	const float maxPitch = 90.0f, minPitch = -90.0f, maxFov = 45.0f, minFov = 1.0f;
	Camera(float aspectRatio);
	Camera(const Camera& copy) = default;

	glm::vec3 getCameraDirection() const;
	glm::vec3 getCameraRight() const;
	glm::vec3 getCameraUp() const;
	glm::vec3 getCameraPosition() const;

	glm::mat4 getViewMatrix() const;
	glm::mat4 getProjectionMatrix() const;


	float getPitch() const;
	float getYaw() const;
	float getFov() const;
	float getAspectRatio() const;
	float getNearPlane() const;
	float getFarPlane() const;


	void setPitch(float pitch);
	void setYaw(float yaw);
	void setFov(float fov);
	void setAspectRatio(float aspectRatio);
	void setNearPlane(float nearPlane);
	void setFarPlane(float farPlane);
	void setCameraPosition(const glm::vec3& cameraPosition);


private:
	glm::vec3 cameraPosition{ 0.0f };
	float pitch = 0.0f, yaw = 0.0f;
	float fov = 45.0f;
	float aspectRatio = 16.0f / 9.0f, nearPlane = 0.1f, farPlane = 100.0f;

	void validateMaxMins();
};