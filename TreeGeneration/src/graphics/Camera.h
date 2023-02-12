#pragma once
#include "Common.h"
#include <optional>
#include <variant>
namespace tgen::graphics {
class Camera {
public:
	static constexpr float maxPitch = 90.0f, minPitch = -90.0f, maxFov = 45.0f, minFov = 1.0f;

	glm::vec3 getCameraDirection() const;
	glm::vec3 getCameraRight() const;
	glm::vec3 getCameraUp() const;

	glm::mat4 getViewMatrix() const;
	glm::mat4 getProjectionMatrix() const;
	glm::mat4 getVP() const;

	float getPitch() const;
	float getYaw() const;
	float getFov() const;


	void setPitch(float pitch);
	void setYaw(float yaw);
	void setFov(float fov);


	std::variant<float, vec4> projection = 16.0f/9.0f;
	glm::vec3 cameraPosition{ 0.0f };
	float nearPlane = 0.01f, farPlane = 10.0f;
	std::optional<vec3> dir;
private:
	float pitch = 0.0f, yaw = 0.0f;
	float fov = 45.0f;
	void validateMaxMins();
};
}