#pragma once

#include "Common.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "TreeGenerator.h"
#include "TreeWorld.h"
#include "graphics/Renderer.h"
#include "graphics/tree/TreeRendererManager.h"
#include "graphics/terrain/TerrainRenderer.h"
#include "PreviewWorld.h"
#include "graphics/CameraPath.h"

namespace tgen::app
{

//shouldn't be included by anything
using namespace tgen::graphics;
using namespace tgen::gen;

using namespace tgen::graphics::gl;
using namespace tgen::graphics::terrain;
using namespace tgen::util;

//Growth Data Presets
static TreeGrowthData DETAILED =
{
	.spread = false,
	.apicalControl = 0.5f,
	.vigorMultiplier = 2.0f,
	.baseLength = .02f,
	.baseRadius = 0.0005f,
	.radiusN = 2.5f,
	.shouldShed = false,
	.shedMultiplier = 0.3f,
	.shedExp = 1.5f,
	.leafMaxChildCount = 5,
	.leafMinOrder = 4,
	.leafDensity = 60.0f,
	.leafSizeMultiplier = 0.4f,
}, DETAILED_WEAK = {
	.apicalControl = 0.46f,
	.vigorMultiplier = 1.2f,
	.baseLength = .02f,
	.baseRadius = 0.0005f,
	.radiusN = 2.5f,
	.shouldShed = false,
	.shedMultiplier = 0.3f,
	.shedExp = 1.5f,
	.leafMaxChildCount = 5,
	.leafMinOrder = 4,
	.leafDensity = 60.0f,
	.leafSizeMultiplier = 0.4f,
}, DETAILED_LOW = {
	.apicalControl = 0.54f,
	.vigorMultiplier = 2.0f,
	.baseLength = .05f,
	.baseRadius = 0.001f,
	.radiusN = 2.3f,
	.shouldShed = false,

	.leafMaxChildCount = 5,
	.leafMinOrder = 4,
	.leafDensity = 120.0f,
	.leafSizeMultiplier = 0.4f,
}, TEST =
{
	.spread = false,
	.apicalControl = 0.5f,
	.vigorMultiplier = 2.0f,
	.baseLength = .02f,
	.baseRadius = 0.0005f,
	.radiusN = 2.5f,
	.shouldShed = true,
	.shedMultiplier = 0.3f,
	.shedExp = 1.5f,
	.leafMaxChildCount = 5,
	.leafMinOrder = 4,
	.leafDensity = 60.0f,
	.leafSizeMultiplier = 0.4f,
};

#include "Questions.inl"

static TreeGrowthData SelectedGrowthData = TEST;

static TreeWorldGrowthData DETAILED_WORLD = {
	.fullExposure = 2.5f,
	.pyramidHeight = 6,
	.a = 0.8f,
	.b = 1.5f,
}, DETAILED_LOW_WORLD = {
	.fullExposure = 2.5f,
	.pyramidHeight = 6,
	.a = 0.8f,
	.b = 1.5f,
};

static std::vector<glm::vec<3, uint8>> presetColors = { {255, 0, 0}, {0, 255, 0} };

struct TreeApplicationData
{
	uint32 width = 1024, height = 1024;
	float mouseSensitivity = 0.1f;
	float cameraSpeed = 0.5f;

	vec3 camPos = vec3(-3.f, .8f, 3.f);
	//vec3 camPos = vec3(-1.63f, 1.18f, .25f);
	//vec3 camPos = vec3(-1.3f, 0.4f, 1.3f);
	float yaw = -45.f, pitch = -0.0f;
	//float yaw = 348.0f, pitch = -15.0f;

	//q2
	//vec3 camPos = vec3(-3.0f, 1.5f, -3.0f);
	//float yaw = 45.0f, pitch = -20.0f;

	float fov = 45.0f;

	bool previewWorld = false;
	uint32 previewAge = 50;
	bool showShadowGrid = false;
	bool shadowOnOnlyBuds = false;
	float shadowCellVisibilityRadius = 10.0f;

	bool showVigor = false;
	bool showOptimalDirs = false;

	BBox worldBbox = BBox(vec3(-2.0f, 0.0f, -2.0f), vec3(2.0f, 4.0f, 2.0f));

	int treeDistributionSeed = 0;
	int treeCount = 100;

	bool renderTerrain = true;
	bool renderBody = true;
	bool renderLeaves = true;
	bool renderBodyShadow = true;
	bool renderLeafShadow = true;
	bool showImgui = true;
	bool perfMetrics = false;

	bool animated = false;
	bool camAnimated = false;
	float camAnimSpeed = 0.2f;
	float paintSize = 0.5f;

	//vec3 camPos = { 0,1,2 };
	std::vector<vec3> camPoints = { {0.5, 1, 2}, {1.53, 0.75, 2.1}, {1.9, 0.75, 1.72}, {2.28, 0.75, 1.35}, {2.29, 1.12, 0.13}, {2.11, 1.19, -0.32},
		{1.76, 1.32, -1.26}, {1,0.9,-2},{0,0.9,-2},{-0.5,0.9,-2},{-1.28,1.23,-2.07},{-1.66,1.25,-1.7},{-2.03,1.28,-1.32},{-2.19, 0.96, -0.36}, {-2.19,0.96,0.13}, {-2.19, 0.96, 0.63},
		{-1.68, 1.26,0.87}, {-1.26, 1.26, 1.21}, {-0.83, 1.27, 1.55}, {-0.5, 1, 2}, {0,1,2} };

};


class TreeApplication
{
public:
	TreeApplication(const TreeApplicationData& appData);

	TreeApplication(const TreeApplication& copy) = delete;

	void execute();

	~TreeApplication() = default;

	TreeApplicationData appData;
	uint32 framesRendered = 0;

	GLFWwindow* window;

	float deltaTime = 0.0f;	// Time between current frame and last frame
	float currentFrame = 0.0f;
	float lastFrame = 0.0f; // Time of last frame

	bool cursorDisabled = true;

	Camera cam;

	ru<TreeWorld> world;
	ru<PreviewWorld> previewWorld;
	ru<TreeGenerator> generator;


	std::vector<ru<TreeRenderer>> treeRenderers;
	ru<TreeRendererManager> treeRendererManager;

	struct TerrainObject
	{
		ru<Terrain> terrain;
		ru<TerrainRenderer> terrainRenderer;

	} terrainObject;

	TreeWorldGrowthData worldGrowthData = DETAILED_LOW_WORLD;

	bool treeSettingsEdited = false;
	bool previewWorldChanged = false;
	bool radiusSettingsEdited = false;
	bool leafSettingsEdited = false;

	rc<CompleteMesh<Vertex, IndexType::UNSIGNED_INT>> leafMesh{};
	rc<CompleteMesh<Vertex, IndexType::UNSIGNED_INT>> leafShadowMesh{};

	rc<Shader> treeBezierShader{};
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
	rc<Shader> animatedBranchShader{};
	rc<Shader> animatedBranchShadowShader{};

	ru<EventSubscriber<TreeWorld::TreeCreatedEvent>> treeCreatedSubscriber;
	ru<EventSubscriber<TreeWorld::TreeCreatedEvent>> treeDestroyedSubscriber;

	graphics::TreeMaterial treeMaterial{};

	rc<Texture> leafTex{};

	graphics::terrain::TerrainRenderer::TerrainMaterial terrainMaterial{};

	rc<Image> worldPresetImage;
	rc<Image> heightMapImage;

	rc<CubemapTexture> skyboxTex{};


	// map editor
	rc<Shader> editorPlaneShader{};
	rc<Shader> editorSphereShader{};
	rc<CompleteMesh<Vertex, IndexType::UNSIGNED_INT>> planeMesh{};
	rc<CompleteMesh<Vertex, IndexType::UNSIGNED_INT>> sphereMesh{};
	bool editingTerrain = false;
	rb<EditableMap> editingMap = nullptr;
	vec3 editingColor;
	vec2 editingPoint;
	void renderEditing(const Camera& cam);

	void drawToMap(const Camera& cam);

	void startEditing(rc<EditableMap> map, vec3 editingColor);


	bool animationRunning = false;
	float camT = 0.0f;
	CameraPath camPath = 1.5f * appData.camPos;
	std::string presetPath = "./Assets/introPreset.png";
	std::chrono::steady_clock::time_point lastGrowth;


	void startFrame();

	void updateScene();

	void drawGrowthDataGUI(GrowthDataId id, TreeGrowthData& growthData, vec3 color);

	void drawGUI();

	void drawScene();

	void endFrame();

	void processInputs();

	void keyInput();

	void mouseInput(const vec2& offset);

	void scrollInput(const vec2& offset);

	void createRenderers(const std::vector<ru<Tree>>& trees, bool updateRenderer);
	TreeRenderer& createRenderer(Tree& tree);
	void removeRenderer(Tree& tree);
	void redistributeTrees();

	TreeWorld& getActiveWorld();

	void checkPreviewWorld();
	void createPreviewWorld();
	void destroyPreviewWorld();

};
}