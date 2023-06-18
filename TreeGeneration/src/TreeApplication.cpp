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
	glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
	//glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

	window = glfwCreateWindow(appData.width, appData.height, "TreeGen", NULL, NULL);

	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return;
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, [](GLFWwindow* window, double xpos, double ypos)
		{
			Input::GetInstance().mouse_callback(window, xpos, ypos);
		});
	glfwSetScrollCallback(window, [](GLFWwindow* window, double xoffset, double yoffset)
		{
			Input::GetInstance().scroll_callback(window, xoffset, yoffset);
		});
	glfwSetWindowSizeCallback(window, [](GLFWwindow* window, int width, int height)
		{
			Input::GetInstance().windowSizeCallback(window, width, height);
		});
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return;
	}
	const char* glsl_version = "#version 460";
	double xpos, ypos;
	int width, height;
	glfwGetCursorPos(window, &xpos, &ypos);
	glfwGetWindowSize(window, &width, &height);
	Input::GetInstance().init(xpos, ypos, ivec2(width, height));

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
	Renderer::getRenderer().viewportSize = Input::GetInstance().getWindowSize();
#pragma region Load_Resources
	{
		leafMesh = ResourceManager::getInstance().objToMesh("./Assets/leafMesh.wobj");
		leafShadowMesh = ResourceManager::getInstance().objToMesh("./Assets/leaf5Plane.wobj");

		sphereMesh = ResourceManager::getInstance().objToMesh("./Assets/sphereMesh.wobj");
		planeMesh = ResourceManager::getInstance().objToMesh("./Assets/planeMesh.wobj");

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

		editorPlaneShader = rm.createShader(
			SHADERS_FOLDER + "editorPlane_vert.glsl",
			SHADERS_FOLDER + "editorPlane_frag.glsl");

		editorSphereShader = rm.createShader(
			SHADERS_FOLDER + "editorSphere_vert.glsl",
			SHADERS_FOLDER + "editorSphere_frag.glsl");

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
		//terrainMaterial.grassTexture = rm.createTexture("./Assets/Textures/terrain/patchy-meadow1_albedo.png", {});
		//terrainMaterial.dirtTexture = rm.createTexture("./Assets/Textures/terrain/dirt.jpg", {});
		//terrainMaterial.normalMap = rm.createTexture("./Assets/Textures/terrain/patchy-meadow1_normal-ogl.png", {}, true, true, false);
		terrainMaterial.dirtTexture = rm.createTexture("./Assets/Textures/terrain/whiteTexture.png", {});
		terrainMaterial.grassTexture = rm.createTexture("./Assets/Textures/terrain/whiteTexture.png", {});
		terrainMaterial.normalMap = rm.createTexture("./Assets/Textures/terrain/whiteTexture.png", {}, true, true, false);

		skyboxTex = rm.createCubemapTexture({ "./Assets/Textures/skybox/posx.jpg", "./Assets/Textures/skybox/negx.jpg",
			"./Assets/Textures/skybox/posy.jpg", "./Assets/Textures/skybox/negy.jpg",
			"./Assets/Textures/skybox/posz.jpg", "./Assets/Textures/skybox/negz.jpg" }, {}, true);


		Renderer::getRenderer().setupSkybox(skyboxTex, skyboxShader);
	}
#pragma endregion Load_Resources

#pragma region Setup_Cam
	assert(appData.camPoints.size() % 3 == 0);
	for (int i = 0; i < appData.camPoints.size(); i += 3)
	{
		const vec3* ptr = &appData.camPoints[i];
		camPath.addNew(1.5f * ptr[0], 1.5f * ptr[1], 1.5f * ptr[2]);
	}
	/*camPath.addNew(2.f * vec3(-1.0f, 0.5f, -0.5f), 2.f * vec3(-0.5f, 0.5f, -1.f), 2.f * vec3(0.0f, 0.5f, -1.f));

	camPath.addNew(2.f * vec3(0.5f, 0.5f, -1.f), 2.f * vec3(1.0f, 0.5f, -0.5f), 2.f * vec3(1.0f, .5f, 0.f));

	camPath.addNew(2.f * vec3(1.0f, 0.5f, 0.5f), 2.f * vec3(0.5f, 0.5f, 1.f), 2.f * vec3(0.f, 0.5f, 1.f));

	camPath.addNew(2.f * vec3(-0.5f, 0.5f, 1.f), 2.f * vec3(-1.f, 0.5f, .5f), 2.f * vec3(-1.f, 0.5f, 0.f));*/
#pragma endregion

#pragma region Setup_TreeGen
	cam.projection = ((float)appData.width) / ((float)appData.height);
	cam.cameraPosition = appData.camPos;
	cam.setFov(appData.fov);
	cam.setYaw(appData.yaw);
	cam.setPitch(appData.pitch);

	generator = std::make_unique<TreeGenerator>();

	world = std::make_unique<TreeWorld>(worldGrowthData, appData.worldBbox, SelectedGrowthData.baseLength);

	//world->newGrowthData(SelectedGrowthData, presetColors[0]);

	//q2

	//intro
	world->newGrowthData(introBlack, introPresetColors[3]);
	world->newGrowthData(introRed, introPresetColors[0]);
	world->newGrowthData(introGreen, introPresetColors[1]);
	world->newGrowthData(introBlue, introPresetColors[2]);

	//heart

	{
		worldPresetImage = ResourceManager::getInstance().readImageFile(presetPath);
		world->setPresetMap(worldPresetImage);
	}

	{
		heightMapImage = ResourceManager::getInstance().readImageFile("./Assets/Textures/terrain/whiteTexture.png");

		TerrainData terrainData = {};
		terrainData.heightMap = heightMapImage;
		terrainData.maxHeight = 0.2f;
		auto worldInfo = world->getWorldInfo();
		terrainData.size = worldInfo.cellSize * vec2(worldInfo.worldSize.x, worldInfo.worldSize.z) * 3.0f;
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
	world->terrain = terrainObject.terrain.get();
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
	previewWorldChanged = true;
	checkPreviewWorld();
#pragma endregion Setup_TreeGen
}

void TreeApplication::execute()
{
	while (!glfwWindowShouldClose(window))
	{
		startFrame();
		processInputs();
		updateScene();
		if (appData.showImgui)
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

	if (appData.camAnimated)
	{
		cam.cameraPosition = camPath.calculate(camT);
		cam.dir = glm::normalize(vec3(0.0f, 0.25f, 0.0f) - cam.cameraPosition);
		camT += deltaTime * appData.camAnimSpeed;
	}

	else if (false)
	{
		camT += deltaTime * 0.04f;
		if (camT > 1.f)
			camT = 0.0f;
		int a = 2;
		auto& growthData = world->getWorldGrowthData().presets.begin()->second;
		switch (a)
		{
		case 0:
			growthData.apicalControl = glm::mix(0.35f, 0.65f, camT);
			break;
		case 1:
			growthData.tropism.y = glm::mix(-1.f, 1.f, camT);
			break;
		case 2:
			growthData.vigorMultiplier = glm::mix(0.5f, 2.5f, camT);
			break;
		}
		previewWorldChanged = true;
	}

	float sinDeltaTime = glm::sin(currentFrame);
	float cosDeltaTime = glm::cos(currentFrame);
	const vec3 lightDir = glm::normalize(vec3(0.4, -0.6, -0.4));
	vec3 lDir = vec3(cosDeltaTime, sinDeltaTime, cosDeltaTime) * lightDir;

	//DirectionalLight::GDirLight.SetDir(lDir);
}

void TreeApplication::drawGrowthDataGUI(GrowthDataId id, TreeGrowthData& growthData, vec3 color)
{
	treeSettingsEdited |= ImGui::Checkbox("Grow", &growthData.grow);
	treeSettingsEdited |= ImGui::Checkbox("Spread", &growthData.spread);
	treeSettingsEdited |= ImGui::SliderFloat("Apical Control", &growthData.apicalControl, 0.0f, 1.0f);
	treeSettingsEdited |= ImGui::SliderFloat("Vigor Multiplier", &growthData.vigorMultiplier, 0.25f, 4.0f);
	//treeSettingsEdited |= ImGui::SliderFloat("Base Length", &growthData.baseLength, 0.01f, 1.0f);

	treeSettingsEdited |= ImGui::SliderAngle("Lateral Angle", &growthData.lateralAngle, 15.0f, 90.0f);

	treeSettingsEdited |= ImGui::SliderFloat3("Tropism Vector", (float*)&growthData.tropism, -1.0f, 1.0f);

	treeSettingsEdited |= ImGui::SliderFloat("Optimal Direction Weight", &growthData.directionWeights.x, 0.0f, 1.0f);
	treeSettingsEdited |= ImGui::SliderFloat("Tropism Weight", &growthData.directionWeights.y, 0.0f, 1.0f);



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

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(color.x, color.y, color.z, 1.0f));
	if (ImGui::Button("Select Color"))
	{
		editingColor = color;
	}
	ImGui::PopStyleColor();
}

void TreeApplication::drawGUI()
{
	ImGui::Begin("Tree Generator");


	if (ImGui::CollapsingHeader("World Growth Data"))
	{
		bool worldSettingsEdited = false;
		auto& worldGrowthData = world->getWorldGrowthData();
		worldSettingsEdited |= ImGui::SliderFloat("Full Exposure Light", &worldGrowthData.fullExposure, 0.5f, 5.0f);
		worldSettingsEdited |= ImGui::SliderInt("Shadow Pyramid Height", &worldGrowthData.pyramidHeight, 1, 20);
		worldSettingsEdited |= ImGui::SliderFloat("Shadow Pyramid Multiplier", &worldGrowthData.a, 0.1f, 3.0f);
		worldSettingsEdited |= ImGui::SliderFloat("Shadow Pyramid Base", &worldGrowthData.b, 1.1f, 3.0f);
		if (worldSettingsEdited)
			world->recalculateLUT();
		treeSettingsEdited |= worldSettingsEdited;
	}
	if (ImGui::CollapsingHeader("Growth Datas"))
	{
		for (auto& [id, preset] : world->getWorldGrowthData().presets)
		{
			ImGui::PushID(("Growth Data " + std::to_string(id)).c_str());
			vec3 color = vec3(world->getGrowthDataColor(id)) / 255.0f;
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(color.x, color.y, color.z, 1.0f));
			if (ImGui::CollapsingHeader(("Tree Growth Data " + std::to_string(id)).c_str()))
			{
				ImGui::PopStyleColor();
				drawGrowthDataGUI(id, world->getWorldGrowthData().presets[id], color);
			}
			else
				ImGui::PopStyleColor();
			ImGui::PopID();
		}
		if (ImGui::Button("Add New Growth Data"))
		{
			world->newGrowthData();
		}
	}

	if (ImGui::CollapsingHeader("Render Options"))
	{

		treeSettingsEdited |= previewWorldChanged = ImGui::Checkbox("Render Tree Preview", &appData.previewWorld);
		if (appData.previewWorld)
			ImGui::SliderInt("Preview Iterations", (int*)&appData.previewAge, 1, 50);


		ImGui::Checkbox("Render Body", &appData.renderBody);
		ImGui::Checkbox("Render Leaves", &appData.renderLeaves);
		ImGui::Checkbox("Render Body Shadow", &appData.renderBodyShadow);
		ImGui::Checkbox("Render Leaf Shadow", &appData.renderLeafShadow);
		ImGui::Checkbox("Render Terrain", &appData.renderTerrain);
		ImGui::Checkbox("Enable Performance Metrics", &appData.perfMetrics);
		ImGui::Checkbox("Show Shadow Grid", &appData.showShadowGrid);
		ImGui::Checkbox("Show Shadow On Only Buds", &appData.shadowOnOnlyBuds);
		ImGui::Checkbox("Show Vigor", &appData.showVigor);
		ImGui::Checkbox("Show Optimal Dirs", &appData.showOptimalDirs);
		ImGui::SliderFloat("Shadow Cell Visibility Radius", &appData.shadowCellVisibilityRadius, 0.5f, 20.0f);

		if (ImGui::Checkbox("Growth Animated", &appData.animated))
		{
			createRenderers(getActiveWorld().getTrees(), true);
			animationRunning = false;
		}
		ImGui::SliderFloat("Animation Speed", &AnimatedTreeRendererManager::resources.animationSpeed, 0.05f, 2.0f);
		if (ImGui::Checkbox("Camera Animated", &appData.camAnimated))
		{
			camT = 0.0f;
		}
	}
	ImGui::Text("World Pos: (%.3f, %.3f, %.3f), Yaw: %.3f, Pitch: %.3f", cam.cameraPosition.x, cam.cameraPosition.y, cam.cameraPosition.z, cam.getYaw(), cam.getPitch());
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
			maxOrder = glm::max(maxOrder, renderer->getTree()->maxOrder);
			ImGui::Text("Tree:%i Branch Count: %u, Bud Count: %u, Leaf Count: %u, Max Order: %u, Root Vigor: %.3f",
				i, renderer->getBranchCount(), renderer->getBudCount(),
				renderer->getLeafCount(), renderer->getTree()->maxOrder,
				renderer->getTree()->root->vigor);
			i++;
		}
		ImGui::Text("Total Branch Count: %u, Total Bud Count: %u, Total Leaf Count: %u, Max Order: %u",
			totBranch, totBud, totLeaf, maxOrder);
	}

	ImGui::Text("World age: %i", world->age);

	if (ImGui::Button("Reset Trees"))
	{
		redistributeTrees();
	}

	ImGui::End();
}

void TreeApplication::renderEditing(const Camera& cam)
{
	// Render a plane with size of the terrain with the edited texture
	{
		gl::GraphicsPipeline MapDrawPipeline("Draw Map", *editorPlaneShader);
		MapDrawPipeline.vertexInputState = planeMesh->inputState;
		MapDrawPipeline.depthState.depthWriteEnable = false;
		MapDrawPipeline.depthState.depthTestEnable = false;
		Cmd::ScopedGraphicsPipeline _(MapDrawPipeline);

		mat4 model = terrainObject.terrain->getTerrainBBox().asModel();
		Cmd::SetUniform("MVP", cam.getVP() * model);

		glBindTextureUnit(editorPlaneShader->getTextureIndex("map"), editingMap->getTexture()->getHandle());


		Cmd::util::BindMesh(*planeMesh);
		Cmd::DrawIndexed(planeMesh->ebo.getSize());
	}
	// Render a sphere at mouse location
	{
		gl::GraphicsPipeline MapDrawPipeline("Draw Editor Sphere", *editorSphereShader);
		MapDrawPipeline.vertexInputState = sphereMesh->inputState;
		MapDrawPipeline.depthState.depthWriteEnable = false;
		MapDrawPipeline.depthState.depthTestEnable = false;
		Cmd::ScopedGraphicsPipeline _(MapDrawPipeline);

		float aspectRatio = 1.f / (appData.width / (float)appData.height);

		mat4 model = glm::scale(glm::translate(glm::mat4(1.0f), vec3(editingPoint.x, 0.0f, editingPoint.y)), appData.paintSize * vec3(aspectRatio, 1.0f, 1.f));
		Cmd::SetUniform("color", editingColor + 0.02f);
		Cmd::SetUniform("MVP", cam.getVP() * model);

		Cmd::util::BindMesh(*sphereMesh);
		Cmd::DrawIndexed(sphereMesh->ebo.getSize());
	}
}

void TreeApplication::drawToMap(const Camera& cam)
{
	gl::RenderColorAttachment attachment{ .texture = editingMap->getTexture(),.clearOnLoad = false };

	RenderInfo renderInfo{ .name = "Map Draw Pass", .colorAttachments = std::span(&attachment, 1) };

	BeginRendering(renderInfo);
	{
		gl::GraphicsPipeline MapDrawPipeline("Draw To Map", *editorSphereShader);
		MapDrawPipeline.vertexInputState = sphereMesh->inputState;
		Cmd::ScopedGraphicsPipeline _(MapDrawPipeline);

		float aspectRatio = 1.f / (appData.width / (float)appData.height);

		mat4 model = glm::scale(glm::translate(glm::mat4(1.0f), vec3(editingPoint.x, 0.0f, editingPoint.y)), appData.paintSize * vec3(aspectRatio, 1.0f, 1.f));

		Cmd::SetUniform("color", editingColor);
		Cmd::SetUniform("MVP", cam.getVP() * model);

		Cmd::util::BindMesh(*sphereMesh);
		Cmd::DrawIndexed(sphereMesh->ebo.getSize());
	}

	editingMap->edited();
	if (previewWorld)
		previewWorld->age = 0;
	EndRendering();
}

void TreeApplication::startEditing(rc<EditableMap> map, vec3 editingColor)
{
	editingPoint = vec2(0.0f);
	editingTerrain = true;
	editingMap = map.get();
	this->editingColor = editingColor;
}

void TreeApplication::drawScene()
{
	DrawScene scene(DirectionalLight::GDirLight);
	const Camera& cam = editingTerrain ? terrainObject.terrain->getTerrainCamera() : this->cam;
	DrawView view{ cam };

	checkPreviewWorld();

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

	if (editingTerrain)
		renderEditing(cam);

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
		for (auto& renderer : treeRenderers)
			renderer->renderVigor(view);
	}
	if (appData.showOptimalDirs)
	{
		for (auto& renderer : treeRenderers)
			renderer->renderOptimalDirection(view);
	}

	if (!appData.perfMetrics && false)
		Renderer::getRenderer().renderBBoxLines(view, *lineShader, world->getBBox(), vec3(1.0f));

	std::vector<vec4> obstacles;
	for (auto& tree : selWorld.getTrees())
	{
		if (tree->age == 0)
			continue;
		obstacles.push_back({ tree->root->startPos, tree->age / 10.0f * 0.1f });
	}

	if (!appData.perfMetrics)
		terrainRenderers[0]->updateTerrainColor(obstacles);

	if (appData.renderTerrain && !editingTerrain && !appData.perfMetrics)
	{
		TerrainRenderer::renderTerrains(terrainRenderers, view, scene);
	}
	if (!editingTerrain && !appData.perfMetrics && false)
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
	treeRenderers.clear();
	world->clear();
	auto points = util::DistributePoints(appData.treeDistributionSeed, appData.treeCount,
		{ appData.worldBbox.min.x, appData.worldBbox.min.z, appData.worldBbox.max.x, appData.worldBbox.max.z });
	//#ifdef _DEBUG
	if (appData.treeCount == 3)
	{
		points.clear();
		points.push_back(vec2(0.0));
		points.push_back(vec2(0.3, 0.0));
		points.push_back(vec2(0.7, 0.0));
	}
	if (appData.treeCount == 2)
	{
		points.clear();
		points.push_back(vec2(0.0));
		points.push_back(vec2(0.0, 0.2));
	}
	if (appData.treeCount == 1)
	{
		points.clear();
		points.push_back(vec2(0.0));
	}
	//#endif
	uint32 i = 0;
	for (auto& point : points)
	{
		uint32 id = world->getGrowthDataFromMap(point);
		generator->createTree(*world, point, id);
		i++;
	}
	//trees.emplace_back(&generator->createTree(*app.world, vec3(0.0f, app.terrainObject.terrain->heightAtWorldPos(vec3(0.0f)), 0.0), app.growthData));
	//trees.emplace_back(&app.generator->createTree(*app.world, vec3(0.2f, app.terrainObject.terrain->heightAtWorldPos(vec3(0.2f, 0.0f, 0.0f)), 0.0f), app.growthData));
	previewWorld = nullptr;
	createRenderers(world->getTrees(), true);
}

TreeWorld& TreeApplication::getActiveWorld()
{
	if (previewWorld)
		return *previewWorld;
	return *world;
}

void TreeApplication::checkPreviewWorld()
{
	if (previewWorldChanged && !appData.previewWorld)
	{
		destroyPreviewWorld();
	}
	else if (appData.previewWorld && (previewWorldChanged || previewWorld == nullptr))
	{
		createPreviewWorld();
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
			createRenderers(previewWorld->getTrees(), false);
			generator->iterateWorld(*previewWorld, appData.previewAge, appData.renderBody || appData.renderBodyShadow);
		}
	}



}

void TreeApplication::createPreviewWorld()
{
	std::cout << "Create preview world" << std::endl;
	previewWorld = std::make_unique<PreviewWorld>(*world);
	createRenderers(previewWorld->getTrees(), false);
	treeCreatedSubscriber = previewWorld->onTreeCreated.subscribe(this, "app", [this](const auto& data)
		{
			createRenderer(data.newTree);
		});
	treeDestroyedSubscriber = previewWorld->onTreeDestroyed.subscribe(this, "app", [this](const auto& data)
		{
			removeRenderer(data.newTree);
		});
}

void TreeApplication::destroyPreviewWorld()
{
	std::cout << "Clear preview world" << std::endl;
	treeCreatedSubscriber = world->onTreeCreated.subscribe(this, "app", [this](const auto& data)
		{
			createRenderer(data.newTree);
		});
	treeDestroyedSubscriber = world->onTreeDestroyed.subscribe(this, "app", [this](const auto& data)
		{
			removeRenderer(data.newTree);
		});
	previewWorld = nullptr;
	createRenderers(world->getTrees(), true);
}

void TreeApplication::createRenderers(const std::vector<ru<Tree>>& trees, bool updateRenderer)
{
	treeRenderers.clear();
	if (!appData.animated)
		treeRendererManager = std::make_unique<StaticTreeRendererManager>();
	else
		treeRendererManager = std::make_unique<AnimatedTreeRendererManager>(*world);
	for (auto& tree : trees)
	{
		auto& created = createRenderer(*tree);
		if (updateRenderer)
			created.updateRenderer();
	}
}

TreeRenderer& TreeApplication::createRenderer(Tree& tree)
{
	if (!appData.animated)
	{
		return *treeRenderers.emplace_back(std::make_unique<StaticTreeRenderer>(&tree));
	}
	else
	{
		return *treeRenderers.emplace_back(std::make_unique<AnimatedTreeRenderer>(&tree));
	}

}

void TreeApplication::removeRenderer(Tree& tree)
{
	for (int i = 0; i < treeRenderers.size(); i++)
		if (tree.id == treeRenderers[i]->getTree()->id)
		{
			treeRenderers.erase(treeRenderers.begin() + i);
			return;
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
	if (input.didResize())
	{
		auto size = input.getWindowSize();
		appData.width = size.x;
		appData.height = size.y;
		cam.projection = ((float)appData.width) / ((float)appData.height);
		Renderer::getRenderer().viewportSize = size;
	}
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
	{
		if (editingTerrain)
		{
			editingTerrain = false;
			editingMap = nullptr;
		}
		else
		{
			startEditing(world->getPresetMap(), vec3(1.0f, 0.0f, 0.0f));
		}
	}
	if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && editingTerrain && cursorDisabled)
	{
		drawToMap(terrainObject.terrain->getTerrainCamera());
	}

	if (ImGui::IsKeyPressed(ImGuiKey_M))
	{
		appData.camAnimated ^= 1;
		camT = 0.0f;
	}
	if (ImGui::IsKeyPressed(ImGuiKey_R))
	{
		camT = 0.0f;
	}


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
	if (!editingTerrain)
	{
		if (!appData.camAnimated)
		{
			cam.setYaw(cam.getYaw() + appData.mouseSensitivity * offset.x);
			cam.setPitch(cam.getPitch() + appData.mouseSensitivity * offset.y);

			appData.yaw = cam.getYaw();
			appData.pitch = cam.getPitch();
		}
	}
	else
	{
		editingPoint += appData.mouseSensitivity * vec2(offset.x, -offset.y) * 0.1f;
	}
}

void TreeApplication::scrollInput(const vec2& offset)
{

	cam.setFov(cam.getFov() - (float)offset.y);
	appData.fov = cam.getFov();
}
#pragma endregion Inputs
}