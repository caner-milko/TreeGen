#include "TreeApplication.h"


#include <iostream>

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include "Input.h"
#include "ResourceManager.h"
#include "Leaf.h"
#include <algorithm>
#include "util/Util.h"
namespace tgen::app
{
TreeApplication::TreeApplication(const TreeApplicationData& appData)
{
#pragma region Setup_GLFW

	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(appData.width, appData.height, "TreeGen", NULL, NULL);

	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return;
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, [](GLFWwindow* window, double xpos, double ypos)
		{
			Input::GetInstance().mouse_callback(window, xpos, ypos);
		});
	glfwSetScrollCallback(window, [](GLFWwindow* window, double xoffset, double yoffset)
		{
			Input::GetInstance().scroll_callback(window, xoffset, yoffset);
		});

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return;
	}
	const char* glsl_version = "#version 460";
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);
	Input::GetInstance().init(xpos, ypos);

#pragma endregion Setup_GLFW

#pragma region ImGui_Setup
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
	//io.ConfigViewportsNoAutoMerge = true;
	//io.ConfigViewportsNoTaskBarIcon = true;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();

	// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);
#pragma endregion ImGui_Setup

	Renderer::getRenderer();

#pragma region Load_Resources
	{
		leafMesh = ResourceManager::getInstance().objToMesh("./Assets/leafMesh.wobj");
		leafShadowMesh = ResourceManager::getInstance().objToMesh("./Assets/leaf5Plane.wobj");
		const std::string SHADERS_FOLDER = "./Assets/Shaders/";
		auto& rm = ResourceManager::getInstance();
		std::string vertex = ResourceManager::getInstance().readTextFile(SHADERS_FOLDER + "shadowPoint_vert.glsl");
		std::string fragment = ResourceManager::getInstance().readTextFile(SHADERS_FOLDER + "shadowPoint_frag.glsl");
		shadowPointShader = rm.createShader(
			SHADERS_FOLDER + "shadowPoint_vert.glsl",
			SHADERS_FOLDER + "shadowPoint_frag.glsl");

		treeBezierShader = rm.createShader(
			SHADERS_FOLDER + "treeBezier_vert.glsl",
			SHADERS_FOLDER + "treeBezier_frag.glsl");

		skyboxShader = rm.createShader(
			SHADERS_FOLDER + "skybox_vert.glsl",
			SHADERS_FOLDER + "skybox_frag.glsl");

		leafShader = rm.createShader(
			SHADERS_FOLDER + "leaf_vert.glsl",
			SHADERS_FOLDER + "leaf_frag.glsl");

		budPointShader = rm.createShader(
			SHADERS_FOLDER + "budPoint_vert.glsl",
			SHADERS_FOLDER + "budPoint_frag.glsl");

		planeShader = rm.createShader(
			SHADERS_FOLDER + "basic_vert.glsl",
			SHADERS_FOLDER + "basic_frag.glsl");

		lineShader = rm.createShader(
			SHADERS_FOLDER + "line_vert.glsl",
			SHADERS_FOLDER + "line_frag.glsl");
		coloredLineShader = rm.createShader(
			SHADERS_FOLDER + "coloredLineShader_vert.glsl",
			SHADERS_FOLDER + "coloredLineShader_frag.glsl");
		terrainShader = rm.createShader(
			SHADERS_FOLDER + "terrain_vert.glsl",
			SHADERS_FOLDER + "terrain_frag.glsl");


		terrainShadowShader = rm.createShader(
			SHADERS_FOLDER + "shadow/terrainShadow_vert.glsl",
			SHADERS_FOLDER + "shadow/terrainShadow_frag.glsl");
		branchShadowShader = rm.createShader(
			SHADERS_FOLDER + "shadow/treeBezierShadow_vert.glsl",
			SHADERS_FOLDER + "shadow/treeBezierShadow_frag.glsl");

		leavesShadowShader = rm.createShader(
			SHADERS_FOLDER + "shadow/leafShadow_vert.glsl",
			SHADERS_FOLDER + "shadow/leafShadow_frag.glsl");

		animatedBranchShader = rm.createShader(
			SHADERS_FOLDER + "treeBezierAnimated_vert.glsl",
			SHADERS_FOLDER + "treeBezierAnimated_frag.glsl");

		Renderer::getRenderer().pointShader = rm.createShader(
			SHADERS_FOLDER + "point_vert.glsl",
			SHADERS_FOLDER + "point_frag.glsl");

		treeMaterial.colorTexture = rm.createTexture("./Assets/Textures/tree/color.jpg", {});
		treeMaterial.normalTexture = rm.createTexture("./Assets/Textures/tree/normal.jpg", {}, true, true, false);
		leafTex = rm.createTexture("./Assets/Textures/tree/leaf5.png", { .wrapping = AddressMode::CLAMP_TO_EDGE });
		terrainMaterial.grassTexture = rm.createTexture("./Assets/Textures/terrain/patchy-meadow1_albedo.png", {});
		terrainMaterial.normalMap = rm.createTexture("./Assets/Textures/terrain/patchy-meadow1_normal-ogl.png", {}, true, true, false);

		skyboxTex = rm.createCubemapTexture({ "./Assets/Textures/skybox/posx.jpg", "./Assets/Textures/skybox/negx.jpg",
			"./Assets/Textures/skybox/posy.jpg", "./Assets/Textures/skybox/negy.jpg",
			"./Assets/Textures/skybox/posz.jpg", "./Assets/Textures/skybox/negz.jpg" }, {}, true);


		Renderer::getRenderer().setupSkybox(skyboxTex, skyboxShader);



	}
#pragma endregion Load_Resources

#pragma region Setup_TreeGen
	cam.projection = ((float)appData.width) / ((float)appData.height);
	cam.cameraPosition = appData.camPos;
	cam.setFov(appData.fov);
	cam.setYaw(appData.yaw);
	cam.setPitch(appData.pitch);

	generator = std::make_unique<TreeGenerator>();

	world = std::make_unique<TreeWorld>(worldGrowthData, appData.worldBbox, baseGrowthData.baseLength);
	world->newGrowthData(DETAILED, presetColors[0]);
	world->newGrowthData(DETAILED_LOW, presetColors[1]);
	{
		worldPresetImage = ResourceManager::getInstance().readImageFile("./Assets/preset.png");
		world->setPresetMap(worldPresetImage);
	}

	{
		heightMapImage = ResourceManager::getInstance().readImageFile("./Assets/Textures/terrain/noiseTexture.png");

		TerrainData terrainData = {};
		terrainData.heightMap = heightMapImage;
		terrainData.maxHeight = 0.2f;
		auto worldInfo = world->getWorldInfo();
		terrainData.size = worldInfo.cellSize * vec2(worldInfo.worldSize.x, worldInfo.worldSize.z);
		terrainData.center = worldInfo.leftBottomCorner +
			worldInfo.cellSize * vec3(worldInfo.worldSize.x, 0.0f, worldInfo.worldSize.z) / 2.0f;
		terrainObject.terrain = std::make_unique<Terrain>(terrainData);
		terrainObject.terrainRenderer = std::make_unique<TerrainRenderer>(*terrainObject.terrain);
		{
			auto& res = TerrainRenderer::resources;
			res.terrainShader = terrainShader;
			res.terrainShadowShader = terrainShadowShader;
			res.material = terrainMaterial;
			res.lineShader = lineShader;
			res.lineVAO = &Renderer::getRenderer().getLineMesh();
			res.camUBO = &Renderer::getRenderer().getCamUBO();
			res.lightUBO = &Renderer::getRenderer().getLightUBO();
		}
		terrainObject.terrainRenderer->update();
	}

	{
		{
			auto& res = TreeRendererManager::resources;
			res.leafMesh = leafMesh;
			res.leafShadowMesh = leafShadowMesh;
			res.cubeMesh = &Renderer::getRenderer().getCubeMesh();
			res.pointMesh = &Renderer::getRenderer().getPointMesh();
			res.lineMesh = &Renderer::getRenderer().getLineMesh();
			res.budPointShader = budPointShader;
			res.coloredLineShader = coloredLineShader;
			res.leafTexture = leafTex;
			res.material = treeMaterial;
			res.camUBO = &Renderer::getRenderer().getCamUBO();
			res.lightUBO = &Renderer::getRenderer().getLightUBO();
			auto& staticRes = StaticTreeRendererManager::resources;
			staticRes.branchShader = treeBezierShader;
			staticRes.leafShader = leafShader;
			staticRes.branchShadowShader = branchShadowShader;
			staticRes.leavesShadowShader = leavesShadowShader;
			auto& animatedRes = AnimatedTreeRendererManager::resources;
			animatedRes.animatedBranchShader = animatedBranchShader;
		}
		redistributeTrees();
	}
#pragma endregion Setup_TreeGen
}

void TreeApplication::execute()
{
	while (!glfwWindowShouldClose(window))
	{
		startFrame();
		processInputs();
		updateScene();
		drawGUI();
		drawScene();
		endFrame();
	}
}

#pragma region Frame
void TreeApplication::startFrame()
{
	treeSettingsEdited = false;
	previewWorldChanged = false;
	radiusSettingsEdited = false;
	leafSettingsEdited = false;
	glfwPollEvents();

	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	currentFrame = (float)glfwGetTime();
	deltaTime = currentFrame - lastFrame;
}

void TreeApplication::updateScene()
{
	if (animationRunning)
	{
		std::chrono::duration<float> diff = std::chrono::steady_clock::now() - lastGrowth;
		float timeSinceLastGrowth = diff.count() * AnimatedTreeRendererManager::resources.animationSpeed;
		if (timeSinceLastGrowth > 1.0f)
		{
			generator->iterateWorld(*world, 1, appData.renderBody || appData.renderBodyShadow);
			lastGrowth = std::chrono::steady_clock::now();
		}
	}


	float sinDeltaTime = glm::sin(currentFrame);
	float cosDeltaTime = glm::cos(currentFrame);
	const vec3 lightDir = glm::normalize(vec3(0.4, -0.6, -0.4));
	vec3 lDir = vec3(cosDeltaTime, sinDeltaTime, cosDeltaTime) * lightDir;

	//DirectionalLight::GDirLight.SetDir(lDir);
}

void TreeApplication::drawGUI()
{
	ImGui::Begin("Tree Generator");

	if (ImGui::CollapsingHeader("Tree Growth Data"))
	{
		bool worldSettingsEdited = false;
		auto& worldGrowthData = world->getWorldGrowthData();
		auto& growthData = world->getWorldGrowthData().presets.begin()->second;
		int32 seed = trees[0]->seed;
		treeSettingsEdited |= ImGui::DragInt("Seed", &seed);
		trees[0]->seed = seed;
		treeSettingsEdited |= ImGui::SliderFloat("Apical Control", &growthData.apicalControl, 0.0f, 1.0f);
		treeSettingsEdited |= ImGui::SliderFloat("Vigor Multiplier", &growthData.vigorMultiplier, 0.25f, 4.0f);
		//treeSettingsEdited |= ImGui::SliderFloat("Base Length", &growthData.baseLength, 0.01f, 1.0f);


		treeSettingsEdited |= ImGui::SliderAngle("Lateral Angle", &growthData.lateralAngle, 15.0f, 90.0f);

		treeSettingsEdited |= ImGui::SliderFloat3("Tropism Vector", (float*)&growthData.tropism, -1.0f, 1.0f);

		treeSettingsEdited |= ImGui::SliderFloat("Optimal Direction Weight", &growthData.directionWeights.x, 0.0f, 1.0f);
		treeSettingsEdited |= ImGui::SliderFloat("Tropism Weight", &growthData.directionWeights.y, 0.0f, 1.0f);

		worldSettingsEdited |= ImGui::SliderFloat("Full Exposure Light", &worldGrowthData.fullExposure, 0.5f, 5.0f);
		worldSettingsEdited |= ImGui::SliderInt("Shadow Pyramid Height", &worldGrowthData.pyramidHeight, 1, 20);
		worldSettingsEdited |= ImGui::SliderFloat("Shadow Pyramid Multiplier", &worldGrowthData.a, 0.1f, 3.0f);
		worldSettingsEdited |= ImGui::SliderFloat("Shadow Pyramid Base", &worldGrowthData.b, 1.1f, 3.0f);

		treeSettingsEdited |= ImGui::Checkbox("Shedding", &growthData.shouldShed);
		treeSettingsEdited |= ImGui::SliderFloat("Shed Multiplier", &growthData.shedMultiplier, 0.0f, 3.0f);
		treeSettingsEdited |= ImGui::SliderFloat("Shed Exp", &growthData.shedExp, 0.5f, 5.0f);

		radiusSettingsEdited |= ImGui::SliderFloat("Branch Radius Min", &growthData.baseRadius, 0.00001f, 0.005f, "%.5f", ImGuiSliderFlags_Logarithmic);
		radiusSettingsEdited |= ImGui::SliderFloat("Branch Radius Power", &growthData.radiusN, 1.0f, 4.0f);
		radiusSettingsEdited |= ImGui::SliderFloat("Branch Curviness", &growthData.branchCurviness, 0.0f, 1.0f);

		leafSettingsEdited |= ImGui::SliderInt("Leaf Max Child Count", &growthData.leafMaxChildCount, 0, 30);
		leafSettingsEdited |= ImGui::SliderInt("Leaf Min Order", &growthData.leafMinOrder, 0, 20);
		leafSettingsEdited |= ImGui::SliderFloat("Leaf Density", &growthData.leafDensity, 0.5f, 150.0f);
		leafSettingsEdited |= ImGui::SliderFloat("Leaf Size Multiplier", &growthData.leafSizeMultiplier, 0.05f, 3.0f);
		leafSettingsEdited |= ImGui::SliderAngle("Leaf Angle", &Leaf::pertubateAngle);

		treeSettingsEdited |= worldSettingsEdited;
		world->recalculateLUT();
	}

	if (ImGui::CollapsingHeader("Render Options"))
	{

		treeSettingsEdited |= previewWorldChanged = ImGui::Checkbox("Render Tree Preview", &appData.previewWorld);
		if (appData.previewWorld)
			ImGui::SliderInt("Preview Iterations", (int*)&appData.previewAge, 1, 15);

		ImGui::Checkbox("Render Body", &appData.renderBody);
		ImGui::Checkbox("Render Leaves", &appData.renderLeaves);
		ImGui::Checkbox("Render Body Shadow", &appData.renderBodyShadow);
		ImGui::Checkbox("Render Leaf Shadow", &appData.renderLeafShadow);
		ImGui::Checkbox("Render Terrain", &appData.renderTerrain);
		ImGui::Checkbox("Show Shadow Grid", &appData.showShadowGrid);
		ImGui::Checkbox("Show Shadow On Only Buds", &appData.shadowOnOnlyBuds);
		ImGui::Checkbox("Show Vigor", &appData.showVigor);
		ImGui::Checkbox("Show Optimal Dirs", &appData.showOptimalDirs);
		ImGui::SliderFloat("Shadow Cell Visibility Radius", &appData.shadowCellVisibilityRadius, 0.5f, 20.0f);
		ImGui::SliderFloat("Animation Speed", &AnimatedTreeRendererManager::resources.animationSpeed, 0.05f, 2.0f);
	}
	ImGui::Text("Frame Rate: %.3f, Frame Time: %.3f ms", ImGui::GetIO().Framerate, 1.0 / ImGui::GetIO().Framerate * 1000.0);

	if (ImGui::CollapsingHeader("Trees"))
	{
		ImGui::SliderInt("Tree Count", &appData.treeCount, 1, 100);
		ImGui::SliderInt("Tree Distribution Seed", &appData.treeDistributionSeed, 0, 100);
		int i = 0;
		int totBranch = 0, totBud = 0, totLeaf = 0;
		uint32 maxOrder = 0;
		for (auto& renderer : treeRenderers)
		{
			totBranch += renderer->getBranchCount();
			totBud += renderer->getBudCount();
			totLeaf += renderer->getLeafCount();
			maxOrder = glm::max(maxOrder, trees[i]->maxOrder);
			ImGui::Text("Tree:%i Branch Count: %u, Bud Count: %u, Leaf Count: %u, Max Order: %u",
				i, renderer->getBranchCount(), renderer->getBudCount(), renderer->getLeafCount(), trees[i]->maxOrder);
			i++;
		}
		ImGui::Text("Total Branch Count: %u, Total Bud Count: %u, Total Leaf Count: %u, Max Order: %u",
			totBranch, totBud, totLeaf, maxOrder);
	}



	if (ImGui::Button("Reset Trees"))
	{
		redistributeTrees();
	}

	ImGui::End();
}

void TreeApplication::drawScene()
{
	DrawScene scene(DirectionalLight::GDirLight);
	const Camera& cam = editingTerrain ? terrainObject.terrain->getTerrainCamera() : this->cam;
	DrawView view{ cam };

	if (previewWorldChanged && !appData.previewWorld)
	{
		std::cout << "Clear preview world" << std::endl;
		previewWorld = nullptr;
		CreateRenderers(world->getTrees(), true);
	}
	if (appData.previewWorld && (previewWorldChanged || previewWorld == nullptr))
	{
		std::cout << "Create preview world" << std::endl;
		previewWorld = std::make_unique<PreviewWorld>(*world);
	}

	if (appData.previewWorld)
	{
		if (treeSettingsEdited)
		{
			std::cout << "Tree settings edited" << std::endl;
			previewWorld->age = 0;
		}
		if (previewWorld->age - world->age != appData.previewAge)
		{
			std::cout << "Recalculate until age" << std::endl;
			previewWorld->ResetToRealWorld();
			CreateRenderers(previewWorld->getTrees(), false);
			generator->iterateWorld(*previewWorld, appData.previewAge, appData.renderBody || appData.renderBodyShadow);
		}
	}


	TreeWorld& selWorld = appData.previewWorld ? *previewWorld : *world;

	if (radiusSettingsEdited)
	{
		for (auto& tree : selWorld.getTrees())
			tree->recalculateBranchs(false);
		for (auto& renderer : treeRenderers)
			renderer->updateRenderer();
	}
	else
	{
		if (leafSettingsEdited)
		{
			for (auto& tree : selWorld.getTrees())
				tree->generateLeaves();
			for (auto& renderer : treeRenderers)
				renderer->updateRenderer();
		}
	}

	std::vector<rb<TreeRenderer>> renderers;
	std::transform(treeRenderers.begin(), treeRenderers.end(), std::back_inserter(renderers), [](auto& a) { return a.get(); });

	rb<TerrainRenderer> terrainRenderers[] = { terrainObject.terrainRenderer.get() };
	DrawView lightView(*scene.light.lightCam);

	Renderer::getRenderer().updateCameraUBO(view);
	Renderer::getRenderer().updateLightUBO(scene);

	Renderer::getRenderer().startShadowPass();

	treeRendererManager->renderTreeShadows(renderers, lightView, appData.renderBodyShadow, appData.renderLeafShadow);
	/*if (renderTerrain)
		TerrainRenderer::renderTerrainShadows(terrainRenderers, lightView);*/

	Renderer::getRenderer().endShadowPass();

	Renderer::getRenderer().beginSwapchain();

	//Renderer::getRenderer().renderTest(view);

	treeRendererManager->renderTrees(renderers, view, scene, appData.renderBody, appData.renderLeaves);

	/*if (appData.showShadowGrid) {
		//world->calculateShadows();
		if (!appData.shadowOnOnlyBuds) {
			std::vector<vec4> cells
				= world->renderShadowCells(cam.getCameraPosition(), cam.getCameraDirection(), cam.getFov(), appData.shadowCellVisibilityRadius);

			renderer.renderShadowPoints(view, *shadowPointShader, cells);
		}
		else {
			std::vector<TreeNode> buds = selTree->AsNodeVector(true);
			renderer.renderShadowsOnBuds(view, *shadowPointShader, *world, buds);
			buds = tree2->AsNodeVector(true);
			renderer.renderShadowsOnBuds(view, *shadowPointShader, *world, buds);
		}
	}*/
	if (appData.showVigor)
	{
		treeRenderers[0]->renderVigor(view);
		treeRenderers[1]->renderVigor(view);
	}
	if (appData.showOptimalDirs)
	{
		treeRenderers[0]->renderOptimalDirection(view);
		treeRenderers[1]->renderOptimalDirection(view);
	}

	Renderer::getRenderer().renderBBoxLines(view, *lineShader, world->getBBox(), vec3(1.0f));

	if (appData.renderTerrain)
	{
		TerrainRenderer::renderTerrains(terrainRenderers, view, scene);
	}
	Renderer::getRenderer().renderSkybox(view);
	Renderer::getRenderer().endSwapchain();

}

void TreeApplication::endFrame()
{
	// Rendering
	ImGui::Render();
	int display_w, display_h;
	glfwGetFramebufferSize(window, &display_w, &display_h);
	glViewport(0, 0, display_w, display_h);
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	// Update and Render additional Platform Windows
	// (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
	//  For this specific demo app we could also call glfwMakeContextCurrent(window) directly)
	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		GLFWwindow* backup_current_context = glfwGetCurrentContext();
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
		glfwMakeContextCurrent(backup_current_context);
	}

	glfwSwapBuffers(window);

	Input::GetInstance().Reset();
	framesRendered++;
	lastFrame = currentFrame;
}
#pragma endregion Frame


void TreeApplication::redistributeTrees()
{
	trees.clear();
	treeRenderers.clear();
	world->clear();
	auto points = util::DistributePoints(appData.treeDistributionSeed, appData.treeCount,
		{ appData.worldBbox.min.x, appData.worldBbox.min.z, appData.worldBbox.max.x, appData.worldBbox.max.z });
	if (appData.treeCount == 2)
	{
		points.clear();
		points.push_back(vec2(0.0));
		points.push_back(vec2(0.0, 0.2));
	}
	uint32 i = 0;
	for (auto& point : points)
	{
		uint32 id = world->getGrowthDataFromMap(point);
		trees.emplace_back(&generator->createTree(*world, vec3(point.x, terrainObject.terrain->heightAtWorldPos(vec3(point.x, 0.0f, point.y)), point.y), id));
		i++;
	}
	//trees.emplace_back(&generator->createTree(*app.world, vec3(0.0f, app.terrainObject.terrain->heightAtWorldPos(vec3(0.0f)), 0.0), app.growthData));
	//trees.emplace_back(&app.generator->createTree(*app.world, vec3(0.2f, app.terrainObject.terrain->heightAtWorldPos(vec3(0.2f, 0.0f, 0.0f)), 0.0f), app.growthData));
	previewWorld = nullptr;
	CreateRenderers(world->getTrees(), true);
}

void TreeApplication::CreateRenderers(const std::vector<ru<Tree>>& trees, bool updateRenderer)
{
	treeRenderers.clear();
	if (!appData.animated)
		treeRendererManager = std::make_unique<StaticTreeRendererManager>();
	else
		treeRendererManager = std::make_unique<AnimatedTreeRendererManager>(*world);
	for (auto& tree : trees)
	{

		if (!appData.animated)
		{
			auto& newRenderer = treeRenderers.emplace_back(std::make_unique<StaticTreeRenderer>(tree.get()));
			if (updateRenderer)
				newRenderer->updateRenderer();
		}
		else
		{
			auto& newRenderer = treeRenderers.emplace_back(std::make_unique<AnimatedTreeRenderer>(tree.get()));
			if (updateRenderer)
				newRenderer->updateRenderer();
		}

	}
}

#pragma region Inputs

void TreeApplication::processInputs()
{
	keyInput();
	Input& input = Input::GetInstance();
	if (input.didMouseMove())
		mouseInput(input.getMouseOffset());
	if (input.didScroll())
		scrollInput(input.getScrollOffset());
}

void TreeApplication::keyInput()
{
	if (ImGui::IsKeyDown(ImGuiKey_Escape))
		glfwSetWindowShouldClose(window, true);
	vec3 camPos = cam.cameraPosition;
	const float cameraSpeed = appData.cameraSpeed * deltaTime;

	if (ImGui::IsKeyDown(ImGuiKey_W))
		camPos += cameraSpeed * cam.getCameraDirection();
	if (ImGui::IsKeyDown(ImGuiKey_S))
		camPos -= cameraSpeed * cam.getCameraDirection();
	if (ImGui::IsKeyDown(ImGuiKey_A))
		camPos -= cam.getCameraRight() * cameraSpeed;
	if (ImGui::IsKeyDown(ImGuiKey_D))
		camPos += cam.getCameraRight() * cameraSpeed;
	if (ImGui::IsKeyDown(ImGuiKey_Space))
		camPos += glm::vec3(0.0f, cameraSpeed, 0.0f);
	if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl))
		camPos += glm::vec3(0.0f, -cameraSpeed, 0.0f);
	if (ImGui::IsKeyPressed(ImGuiKey_K))
		editingTerrain = !editingTerrain;
	if (ImGui::IsKeyDown(ImGuiKey_L))
	{
		auto& light = DirectionalLight::GDirLight;
		light.SetDir(cam.getCameraDirection());
		light.lightCam->cameraPosition = cam.cameraPosition;
	}
	if (ImGui::IsKeyPressed(ImGuiKey_F, false))
	{
		if (appData.animated)
		{
			animationRunning = !animationRunning;
		}
		else
			generator->iterateWorld(*world, 1, appData.renderBody || appData.renderBodyShadow);
	}

	if (ImGui::IsKeyPressed(ImGuiKey_E, false))
	{
		if (cursorDisabled)
		{
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
		else
		{
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		}
		cursorDisabled = !cursorDisabled;
	}

	cam.cameraPosition = camPos;
}

void TreeApplication::mouseInput(const vec2& offset)
{
	if (framesRendered == 0) // initially set to true
	{
		return;
	}
	if (!cursorDisabled)
		return;

	cam.setYaw(cam.getYaw() + appData.mouseSensitivity * offset.x);
	cam.setPitch(cam.getPitch() + appData.mouseSensitivity * offset.y);

	appData.yaw = cam.getYaw();
	appData.pitch = cam.getPitch();
}

void TreeApplication::scrollInput(const vec2& offset)
{

	cam.setFov(cam.getFov() - (float)offset.y);
	appData.fov = cam.getFov();
}
#pragma endregion Inputs
}