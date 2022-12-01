#pragma once

#include "Definitions.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "./TreeGenerator.h"
#include "./TreeWorld.h"
#include "./opengl/Renderer.h"
#include "./opengl/Texture.h"
#include "./opengl/Shader.h"

struct TreeApplicationData {
	uint32 width = 1600, height = 900;
	float mouseSensitivity = 0.1f;
	float cameraSpeed = 2.5f;

	vec3 camPos = vec3(-1.0f, 0.0f, 0.0f);
	float yaw = 0.0f, pitch = 0.0f;
	float fov = 45.0f;

	bool renderPreviewTree = true;
	bool showShadowGrid = false;
	float shadowCellVisibilityRadius = 10.0f;

	float baseTreeLength = 0.4f;

	vec3 worldSize = vec3(25.0f, 50.0f, 25.0f);

};


class TreeApplication {
public:
	TreeApplication(const TreeApplicationData& appData);

	TreeApplication(const TreeApplication& copy) = delete;

	void execute();

	~TreeApplication() = default;
private:
	TreeApplicationData appData;
	uint32 framesRendered = 0;

	GLFWwindow* window;

	float deltaTime = 0.0f;	// Time between current frame and last frame
	float currentFrame = 0.0f;
	float lastFrame = 0.0f; // Time of last frame

	bool cursorDisabled = true;

	Camera cam;

	std::unique_ptr<TreeWorld> world;
	std::unique_ptr<TreeGenerator> generator;
	Tree* tree;
	Tree* tree2;

	std::unique_ptr<Tree> previewTree;

	Renderer renderer;

	bool treeSettingsEdited = false;
	bool treePreviewChanged = false;
	bool leafSettingsEdited = false;

	bool growTree1 = true;
	bool growTree2 = true;



	Shader* treeBezierShader;
	Shader* leafShader;
	Shader* shadowPointShader;
	Shader* skyboxShader;
	Shader* planeShader;

	Texture* barkTex;
	Texture* leafTex;

	CubemapTexture* skyboxTex;

	void startFrame();

	void drawGUI();

	void drawScene();

	void endFrame();

	void processInputs();

	void keyInput();

	void mouseInput(const vec2& offset);

	void scrollInput(const vec2& offset);
};