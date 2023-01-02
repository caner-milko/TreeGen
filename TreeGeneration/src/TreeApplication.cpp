#include "TreeApplication.h"


#include <iostream>

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include "Input.h"
#include "opengl/ResourceManager.h"
#include "Leaf.h"
TreeApplication::TreeApplication(const TreeApplicationData& appData) : cam(appData.width / (float)appData.height)
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
	glfwSetCursorPosCallback(window, [](GLFWwindow* window, double xpos, double ypos) {
		Input::GetInstance().mouse_callback(window, xpos, ypos);
		});
	glfwSetScrollCallback(window, [](GLFWwindow* window, double xoffset, double yoffset) {
		Input::GetInstance().scroll_callback(window, xoffset, yoffset);
		});

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return;
	}
	const char* glsl_version = "#version 460";
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

	renderer.init();

#pragma region Load_Resources

	shadowPointShader = ResourceManager::getInstance().loadShader("shadowPoint_shader",
		"./Assets/Shaders/shadowPoint_vert.glsl", "./Assets/Shaders/shadowPoint_frag.glsl");
	treeBezierShader = ResourceManager::getInstance().loadShader("treeQuadMarch_shader",
		"./Assets/Shaders/treeBezier_vert.glsl", "./Assets/Shaders/treeBezier_frag.glsl");
	skyboxShader = ResourceManager::getInstance().loadShader("skybox_shader",
		"./Assets/Shaders/skybox_vert.glsl", "./Assets/Shaders/skybox_frag.glsl");
	leafShader = ResourceManager::getInstance().loadShader("leaf_shader",
		"./Assets/Shaders/leaf_vert.glsl", "./Assets/Shaders/leaf_frag.glsl");
	budPointShader = ResourceManager::getInstance().loadShader("bud_point_shader",
		"./Assets/Shaders/budPoint_vert.glsl", "./Assets/Shaders/budPoint_frag.glsl");
	planeShader = ResourceManager::getInstance().loadShader("plane_shader",
		"./Assets/Shaders/basic_vert.glsl", "./Assets/Shaders/basic_frag.glsl");
	lineShader = ResourceManager::getInstance().loadShader("line_shader",
		"./Assets/Shaders/line_vert.glsl", "./Assets/Shaders/line_frag.glsl");
	coloredLineShader = ResourceManager::getInstance().loadShader("colored_line_shader",
		"./Assets/Shaders/coloredLineShader_vert.glsl", "./Assets/Shaders/coloredLineShader_frag.glsl");


	barkTex = ResourceManager::getInstance().loadTexture("bark_texture", "./Assets/Textures/bark.jpg");
	leafTex = ResourceManager::getInstance().loadTexture("leaf_texture", "./Assets/Textures/leaf2.png", TextureWrapping::CLAMP_TO_EDGE);

	skyboxTex = ResourceManager::getInstance().loadCubemapTexture("skybox", "./Assets/Textures/skybox/posx.jpg", "./Assets/Textures/skybox/negx.jpg",
		"./Assets/Textures/skybox/posy.jpg", "./Assets/Textures/skybox/negy.jpg",
		"./Assets/Textures/skybox/posz.jpg", "./Assets/Textures/skybox/negz.jpg");


	renderer.setupSkybox(skyboxTex, skyboxShader);

#pragma endregion Load_Resources

#pragma region Setup_TreeGen
	cam.setCameraPosition(appData.camPos);
	cam.setFov(appData.fov);
	cam.setYaw(appData.yaw);
	cam.setPitch(appData.pitch);

	generator = std::make_unique<TreeGenerator>();


	world = std::make_unique<TreeWorld>(appData.worldBbox, growthData.baseLength);

	tree = generator->createTree(*world, vec3(0.0f), growthData);
	tree2 = generator->createTree(*world, vec3(0.2f, 0.0f, 0.0f), growthData);


	TreeRendererResources resources{ .quadVAO = &renderer.getQuadVAO(), .cubeVAO = &renderer.getCubeVAO(), .pointVAO = &renderer.getPointVAO(), .lineVAO = &renderer.getLineVAO(),
		.branchShader = treeBezierShader, .leafShader = leafShader, .budPointShader = budPointShader, .coloredLineShader = coloredLineShader,
		.leafTexture = leafTex, .barkTexture = barkTex, };

	treeRenderer1 = std::make_unique<TreeRenderer>(*tree, resources);
	treeRenderer2 = std::make_unique<TreeRenderer>(*tree2, resources);
	treeRenderer1->updateRenderer();
	treeRenderer2->updateRenderer();
#pragma endregion Setup_TreeGen
}

void TreeApplication::execute()
{
	while (!glfwWindowShouldClose(window)) {
		startFrame();
		processInputs();
		drawGUI();
		drawScene();
		endFrame();
	}
}


#pragma region Frame
void TreeApplication::startFrame()
{
	renderer.startFrame();

	treeSettingsEdited = false;
	treePreviewChanged = false;
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

void TreeApplication::drawGUI()
{
	ImGui::Begin("Tree Generator");

	if (ImGui::CollapsingHeader("Tree Growth Data")) {
		//shadows
		float fullExposure = 3.0f;
		int pyramidHeight = 5;
		float a = 0.5f;
		//b > 1
		float b = 2.0f;

		int32 seed = tree->seed;
		treeSettingsEdited |= ImGui::DragInt("Seed", &seed);
		tree->seed = seed;
		treeSettingsEdited |= ImGui::SliderFloat("Apical Control", &growthData.apicalControl, 0.0f, 1.0f);
		treeSettingsEdited |= ImGui::SliderFloat("Vigor Multiplier", &growthData.vigorMultiplier, 0.25f, 4.0f);
		treeSettingsEdited |= ImGui::SliderFloat("Base Length", &growthData.baseLength, 0.01f, 1.0f);


		treeSettingsEdited |= ImGui::SliderAngle("Lateral Angle", &growthData.lateralAngle, 15.0f, 90.0f);

		treeSettingsEdited |= ImGui::SliderFloat3("Tropism Vector", (float*)&growthData.tropism, -1.0f, 1.0f);

		treeSettingsEdited |= ImGui::SliderFloat("Optimal Direction Weight", &growthData.directionWeights.x, 0.0f, 1.0f);
		treeSettingsEdited |= ImGui::SliderFloat("Tropism Weight", &growthData.directionWeights.y, 0.0f, 1.0f);

		treeSettingsEdited |= ImGui::SliderFloat("Full Exposure Light", &growthData.fullExposure, 0.5f, 5.0f);
		treeSettingsEdited |= ImGui::SliderInt("Shadow Pyramid Height", &growthData.pyramidHeight, 1, 20);
		treeSettingsEdited |= ImGui::SliderFloat("Shadow Pyramid Multiplier", &growthData.a, 0.1f, 3.0f);
		treeSettingsEdited |= ImGui::SliderFloat("Shadow Pyramid Base", &growthData.b, 1.1f, 3.0f);

		treeSettingsEdited |= ImGui::Checkbox("Shedding", &growthData.shouldShed);
		treeSettingsEdited |= ImGui::SliderFloat("Shed Multiplier", &growthData.shedMultiplier, 0.0f, 3.0f);
		treeSettingsEdited |= ImGui::SliderFloat("Shed Exp", &growthData.shedExp, 0.5f, 5.0f);



		radiusSettingsEdited |= ImGui::SliderFloat("Branch Radius Min", &growthData.baseRadius, 0.00001f, 0.005f, "%.5f", ImGuiSliderFlags_Logarithmic);
		radiusSettingsEdited |= ImGui::SliderFloat("Branch Radius Power", &growthData.radiusN, 1.0f, 4.0f);
		radiusSettingsEdited |= ImGui::SliderFloat("Branch Curviness", &growthData.branchCurviness, 0.0f, 1.0f);

		leafSettingsEdited |= ImGui::SliderInt("Leaf Max Child Count", &growthData.leafMaxChildCount, 0, 30);
		leafSettingsEdited |= ImGui::SliderInt("Leaf Min Order", &growthData.leafMinOrder, 0, 20);
		leafSettingsEdited |= ImGui::SliderFloat("Leaf Density", &growthData.leafDensity, 0.5f, 30.0f);
		leafSettingsEdited |= ImGui::SliderFloat("Leaf Size Multiplier", &growthData.leafSizeMultiplier, 0.05f, 3.0f);
		leafSettingsEdited |= ImGui::SliderAngle("Leaf Angle", &Leaf::pertubateAngle);

		ImGui::Checkbox("Render Body", &renderBody);
		ImGui::Checkbox("Render Leaves", &renderLeaves),

			tree->growthData = growthData;
		tree2->growthData = growthData;
	}

	if (ImGui::CollapsingHeader("Render Options")) {

		treeSettingsEdited |= treePreviewChanged = ImGui::Checkbox("Render Tree Preview", &appData.renderPreviewTree);
		ImGui::Checkbox("Show Shadow Grid", &appData.showShadowGrid);
		ImGui::Checkbox("Show Shadow On Only Buds", &appData.shadowOnOnlyBuds);
		ImGui::Checkbox("Show Vigor", &appData.showVigor);
		ImGui::Checkbox("Show Optimal Dirs", &appData.showOptimalDirs);
		ImGui::SliderFloat("Shadow Cell Visibility Radius", &appData.shadowCellVisibilityRadius, 0.5f, 20.0f);
	}
	ImGui::Text("Frame time: %.3f", ImGui::GetIO().Framerate);
	ImGui::Text("Tree1 Branch Count: %u, Bud Count: %u, Leaf Count: %u, Max Order: %u", treeRenderer1->getBranchCount(), treeRenderer1->getBudCount(), treeRenderer1->getLeafCount(), tree->maxOrder);
	ImGui::Checkbox("Grow Tree 1", &growTree1);
	ImGui::Checkbox("Grow Tree 2", &growTree2);
	if (ImGui::Button("Reset Trees")) {
		world->removeTree(*tree);
		world->removeTree(*tree2);
		tree = generator->createTree(*world, vec3(0.0f, 0.0f, 0.0f), growthData);
		tree2 = generator->createTree(*world, vec3(0.2f, 0.0f, 0.0f), growthData);
		previewTree = nullptr;
	}
	ImGui::End();
}

void TreeApplication::drawScene()
{
	renderer.startDraw();
	DrawView view{ cam };


	//renderer.renderPlane(view, planeShader, glm::scale(glm::rotate(glm::translate(glm::mat4(1.0), vec3(0.0f, 0.0f, -2.5f)), PI / 2.0f, vec3(1.0f, 0.0f, 0.0f)), vec3(5.0f)));


	if (treePreviewChanged && !appData.renderPreviewTree) {
		previewTree.reset();
		treeRenderer1 = std::make_unique<TreeRenderer>(*tree, treeRenderer1->resources);
		treeRenderer1->updateRenderer();
	}

	if (appData.renderPreviewTree) {
		if (previewTree == nullptr || treeSettingsEdited) {
			previewTree = std::make_unique<Tree>(*tree);
			generator->growTree(*previewTree);
			treeRenderer1 = std::make_unique<TreeRenderer>(*previewTree, treeRenderer1->resources);
			treeRenderer1->updateRenderer();
		}
		previewTree->growthData = tree->growthData;
	}


	Tree* selTree = appData.renderPreviewTree ? previewTree.get() : tree;

	if (radiusSettingsEdited) {
		selTree->recalculateBranchs();
		tree2->recalculateBranchs();
		treeRenderer1->updateRenderer();
		treeRenderer2->updateRenderer();
	}
	else {
		if (leafSettingsEdited) {
			selTree->generateLeaves();
			tree2->generateLeaves();
			treeRenderer1->updateRenderer();
			treeRenderer2->updateRenderer();
		}
	}

	treeRenderer1->renderTree(view, renderBody, renderLeaves);
	treeRenderer2->renderTree(view, renderBody, renderLeaves);

	if (appData.showShadowGrid) {
		world->calculateShadows();
		if (!appData.shadowOnOnlyBuds) {
			std::vector<vec4> cells
				= world->renderShadowCells(cam.getCameraPosition(), cam.getCameraDirection(), cam.getFov(), appData.shadowCellVisibilityRadius);

			renderer.renderShadowPoints(view, shadowPointShader, cells);
		}
		else {
			std::vector<TreeNode> buds = selTree->AsNodeVector(true);
			renderer.renderShadowsOnBuds(view, shadowPointShader, *world, buds);
			buds = tree2->AsNodeVector(true);
			renderer.renderShadowsOnBuds(view, shadowPointShader, *world, buds);
		}
	}
	if (appData.showVigor) {
		treeRenderer1->renderVigor(view);
		treeRenderer2->renderVigor(view);
	}
	if (appData.showOptimalDirs) {
		treeRenderer1->renderOptimalDirection(view);
		treeRenderer2->renderOptimalDirection(view);
	}

	renderer.renderBBoxLines(view, lineShader, world->getBBox(), vec3(1.0f));

	renderer.renderSkybox(view);
	renderer.endDraw();
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
	renderer.endFrame();
	framesRendered++;
	lastFrame = currentFrame;
}
#pragma endregion Frame


#pragma region Inputs

void TreeApplication::processInputs()
{
	keyInput();
	Input& input = Input::GetInstance();
	if (input.didMouseMove())
		mouseInput(input.getMouseOffset());
	if (input.didScroll())
		mouseInput(input.getScrollOffset());
}

void TreeApplication::keyInput()
{
	if (ImGui::IsKeyDown(ImGuiKey_Escape))
		glfwSetWindowShouldClose(window, true);
	vec3 camPos = cam.getCameraPosition();
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

	if (ImGui::IsKeyPressed(ImGuiKey_F, false)) {
		if (growTree1) {
			generator->growTree(*tree);
			treeSettingsEdited = true;
			if (!appData.renderPreviewTree) {
				treeRenderer1->updateRenderer();
			}
		}
		if (growTree2) {
			generator->growTree(*tree2);
			treeRenderer2->updateRenderer();

		}

	}

	if (ImGui::IsKeyPressed(ImGuiKey_E, false)) {
		if (cursorDisabled) {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
		else {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		}
		cursorDisabled = !cursorDisabled;
	}

	cam.setCameraPosition(camPos);
}


void TreeApplication::mouseInput(const vec2& offset) {
	if (framesRendered == 0) // initially set to true
	{
		return;
	}
	if (!cursorDisabled)
		return;

	cam.setYaw(cam.getYaw() + appData.mouseSensitivity * offset.x);
	cam.setPitch(cam.getPitch() + appData.mouseSensitivity * offset.y);

	appData.yaw = cam.getYaw();;
	appData.pitch = cam.getPitch();
}

void TreeApplication::scrollInput(const vec2& offset) {

	cam.setFov(cam.getFov() - (float)offset.y);
	appData.fov = cam.getFov();
}
#pragma endregion Inputs