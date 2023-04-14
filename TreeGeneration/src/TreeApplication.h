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
};

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
	uint32 width = 1600, height = 900;
	float mouseSensitivity = 0.1f;
	float cameraSpeed = 0.5f;

	//vec3 camPos = vec3(-1.0f, 0.0f, 0.0f);
	//float yaw = 0.0f, pitch = 0.0f;
	//float fov = 45.0f;
	// 
	vec3 camPos = vec3(-2.0f, 0.5f, 2.0f);
	float yaw = -45.0f, pitch = 0.0f;
	float fov = 45.0f;


	bool previewWorld = false;
	uint32 previewAge = 1;
	bool showShadowGrid = false;
	bool shadowOnOnlyBuds = false;
	float shadowCellVisibilityRadius = 10.0f;

	bool showVigor = false;
	bool showOptimalDirs = false;

	BBox worldBbox = BBox(vec3(-2.0f, 0.0f, -2.0f), vec3(2.0f, 4.0f, 2.0f));

	int treeDistributionSeed = 0;
	int treeCount = 10;

	bool renderTerrain = true;
	bool renderBody = true;
	bool renderLeaves = true;
	bool renderBodyShadow = true;
	bool renderLeafShadow = true;

	bool animated = true;
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

	ru<EventSubscriber<TreeWorld::TreeCreatedEvent>> treeCreatedSubscriber;
	ru<EventSubscriber<TreeWorld::TreeCreatedEvent>> treeDestroyedSubscriber;

	graphics::TreeMaterial treeMaterial{};

	rc<Texture> leafTex{};

	graphics::terrain::TerrainRenderer::TerrainMaterial terrainMaterial{};

	rc<Image> worldPresetImage;
	rc<Image> heightMapImage;

	rc<CubemapTexture> skyboxTex{};

	bool editingTerrain = false;

	bool animationRunning = false;
	std::chrono::steady_clock::time_point lastGrowth;

	void startFrame();

	void updateScene();

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