#pragma once

#include "Common.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "TreeGenerator.h"
#include "TreeWorld.h"
#include "graphics/Renderer.h"
#include "graphics/TreeRenderer.h"
#include "graphics/terrain/TerrainRenderer.h"
namespace tgen::app {
//shouldn't be included by anything
using namespace tgen::graphics;
using namespace tgen::gen;

using namespace tgen::graphics::gl;
using namespace tgen::graphics::terrain;
using namespace tgen::util;
struct TreeApplicationData {
	uint32 width = 1600, height = 900;
	float mouseSensitivity = 0.1f;
	float cameraSpeed = 0.5f;

	vec3 camPos = vec3(-1.0f, 0.0f, 0.0f);
	float yaw = 0.0f, pitch = 0.0f;
	float fov = 45.0f;

	bool renderPreviewTree = false;
	bool showShadowGrid = false;
	bool shadowOnOnlyBuds = false;
	float shadowCellVisibilityRadius = 10.0f;

	bool showVigor = false;
	bool showOptimalDirs = false;

	BBox worldBbox = BBox(vec3(-2.0f, -0.1f, -2.0f), vec3(2.0f, 4.0f, 2.0f));

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
	std::unique_ptr<TreeRenderer> treeRenderer1;
	std::unique_ptr<TreeRenderer> treeRenderer2;

	std::unique_ptr<Terrain> terrain;
	std::unique_ptr<TerrainRenderer> terrainRenderer;

	std::unique_ptr<Tree> previewTree;

	TreeGrowthData growthData{ .baseLength = 0.02f };

	bool treeSettingsEdited = false;
	bool treePreviewChanged = false;
	bool radiusSettingsEdited = false;
	bool leafSettingsEdited = false;

	bool growTree1 = true;
	bool growTree2 = true;

	bool renderBody = true;
	bool renderLeaves = true;


	rc<gl::Shader> treeBezierShader{};
	rc<Shader> leafShader{};
	rc<Shader> shadowPointShader{};
	rc<Shader> skyboxShader{};
	rc<Shader> planeShader{};
	rc<Shader> lineShader{};
	rc<Shader> budPointShader{};
	rc<Shader> coloredLineShader{};
	rc<Shader> terrainShader{};
	rc<Shader> terrainShadowShader{};
	rc<Shader> branchShadowShader{};
	rc<Shader> leavesShadowShader{};

	graphics::TreeMaterial treeMaterial{};

	rc<Texture> leafTex{};
	rc<Texture> grassTex{};

	rc<Image> heightMapImage;

	rc<CubemapTexture> skyboxTex{};


	void startFrame();

	void updateScene();

	void drawGUI();

	void drawScene();

	void endFrame();

	void processInputs();

	void keyInput();

	void mouseInput(const vec2& offset);

	void scrollInput(const vec2& offset);
};
}