#include "TreeApplication.h"


#include <iostream>

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include "Input.h"
#include "opengl/ResourceManager.h"
TreeApplication::TreeApplication(const TreeApplicationData& appData) : cam(appData.width / (float)appData.height)
{
	cam.setCameraPosition(appData.camPos);
	cam.setFov(appData.fov);
	cam.setYaw(appData.yaw);
	cam.setPitch(appData.pitch);

	generator = std::make_unique<TreeGenerator>();

	vec3 leftCenter = appData.worldSize / -2.0f;
	leftCenter.y = 0.0f;

	world = std::make_unique<TreeWorld>(appData.worldSize / appData.baseTreeLength * 2.0f, leftCenter, appData.baseTreeLength / 2.0f);

	tree = generator->createTree(*world, vec3(0.0f));
	tree2 = generator->createTree(*world, vec3(2.0f, 0.0f, 0.0f));

	tree->growthData.baseLength = appData.baseTreeLength;
	tree2->growthData.baseLength = appData.baseTreeLength;

	tree->init();
	tree2->init();

#pragma region Setup_GLFW

	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
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
	const char* glsl_version = "#version 130";
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
		"./Assets/Shaders/bbox_vert.glsl", "./Assets/Shaders/treeBezier_frag.glsl");
	skyboxShader = ResourceManager::getInstance().loadShader("skybox_shader",
		"./Assets/Shaders/skybox_vert.glsl", "./Assets/Shaders/skybox_frag.glsl");
	leafShader = ResourceManager::getInstance().loadShader("leaf_shader",
		"./Assets/Shaders/leaf_vert.glsl", "./Assets/Shaders/leaf_frag.glsl");
	planeShader = ResourceManager::getInstance().loadShader("plane_shader",
		"./Assets/Shaders/bbox_vert.glsl", "./Assets/Shaders/test_frag.glsl");

	barkTex = ResourceManager::getInstance().loadTexture("bark_texture", "./Assets/Textures/bark.jpg");
	leafTex = ResourceManager::getInstance().loadTexture("leaf_texture", "./Assets/Textures/leaf.png", TextureWrapping::CLAMP_TO_EDGE);

	skyboxTex = ResourceManager::getInstance().loadCubemapTexture("skybox", "./Assets/Textures/skybox/posx.jpg", "./Assets/Textures/skybox/negx.jpg",
		"./Assets/Textures/skybox/posy.jpg", "./Assets/Textures/skybox/negy.jpg",
		"./Assets/Textures/skybox/posz.jpg", "./Assets/Textures/skybox/negz.jpg");


	renderer.setupSkybox(skyboxTex, skyboxShader);

#pragma endregion Load_Resources
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
		TreeGrowthData& growthData = tree->growthData;

		//shadows
		float fullExposure = 3.0f;
		int pyramidHeight = 5;
		float a = 0.5f;
		//b > 1
		float b = 2.0f;

		treeSettingsEdited |= ImGui::SliderFloat("Apical Control", &growthData.apicalControl, 0.0f, 1.0f);
		treeSettingsEdited |= ImGui::SliderFloat("Base Length", &growthData.baseLength, 0.1f, 3.0f);

		treeSettingsEdited |= ImGui::SliderFloat("Perception Radius", &growthData.perceptionRadius, 0.5f, 10.0f);
		treeSettingsEdited |= ImGui::SliderAngle("Perception Angle", &growthData.perceptionAngle, 15.0f, 90.0f);

		treeSettingsEdited |= ImGui::SliderAngle("Lateral Angle", &growthData.lateralAngle, 15.0f, 90.0f);

		treeSettingsEdited |= ImGui::SliderFloat3("Tropism Vector", (float*)&growthData.tropism, -1.0f, 1.0f);

		bool changed = ImGui::SliderFloat("Original Direction Weight", &growthData.directionWeights.x, 0.0f, 1.0f);
		changed = ImGui::SliderFloat("Optimal Direction Weight", &growthData.directionWeights.y, 0.0f, 1.0f) || changed;
		changed = ImGui::SliderFloat("Tropism Weight", &growthData.directionWeights.z, 0.0f, 1.0f) || changed;
		if (changed)
			growthData.directionWeights = glm::normalize(growthData.directionWeights);

		treeSettingsEdited |= changed;
		treeSettingsEdited |= ImGui::SliderFloat("Full Exposure Light", &growthData.fullExposure, 0.5f, 5.0f);
		treeSettingsEdited |= ImGui::SliderInt("Shadow Pyramid Height", &growthData.pyramidHeight, 1, 10);
		treeSettingsEdited |= ImGui::SliderFloat("Shadow Pyramid Multiplier", &growthData.a, 0.1f, 3.0f);
		treeSettingsEdited |= ImGui::SliderFloat("Shadow Pyramid Base", &growthData.b, 1.1f, 3.0f);

		leafSettingsEdited |= ImGui::SliderFloat("Leaf Max Width", &growthData.leafMaxWidth, 0.01f, 2.0f);
		leafSettingsEdited |= ImGui::SliderFloat("Leaf Density", &growthData.leafDensity, 0.5f, 20.0f);

		tree2->growthData = growthData;



	}

	if (ImGui::CollapsingHeader("Render Options")) {

		treeSettingsEdited |= treePreviewChanged = ImGui::Checkbox("Render Tree Preview", &appData.renderPreviewTree);
		ImGui::Checkbox("Show Shadow Grid", &appData.showShadowGrid);
		ImGui::SliderFloat("Shadow Cell Visibility Radius", &appData.shadowCellVisibilityRadius, 0.5f, 20.0f);
	}
	ImGui::Checkbox("Grow Tree 1", &growTree1);
	ImGui::Checkbox("Grow Tree 2", &growTree2);
	ImGui::End();
}

void TreeApplication::drawScene()
{
	DrawView view{ cam };

	renderer.renderPlane(view, planeShader, glm::scale(glm::rotate(glm::translate(glm::mat4(1.0), vec3(0.0f, 0.0f, -2.5f)), PI / 2.0f, vec3(1.0f, 0.0f, 0.0f)), vec3(5.0f)));


	if (treePreviewChanged && !appData.renderPreviewTree) {
		previewTree.reset();
	}

	if (appData.renderPreviewTree) {
		if (previewTree == nullptr || treeSettingsEdited || (previewTree != nullptr && previewTree->age != tree->age + 1)) {
			previewTree = std::make_unique<Tree>(*tree);
			generator->growTree(*previewTree);
		}
	}

	Tree* selTree = appData.renderPreviewTree ? previewTree.get() : tree;

	renderer.renderTree2(view, barkTex, treeBezierShader, selTree->getBranchs());

	renderer.renderTree2(view, barkTex, treeBezierShader, tree2->getBranchs());

	if (leafSettingsEdited) {
		selTree->generateLeaves();
		tree2->generateLeaves();
	}

	renderer.renderLeaves(view, leafTex, leafShader, selTree->getBranchs());

	renderer.renderLeaves(view, leafTex, leafShader, tree2->getBranchs());

	renderer.renderSkybox(view);

	if (appData.showShadowGrid) {
		world->calculateShadows();
		//std::vector<std::tuple<vec3, float>> cells = world->renderShadowCells(cam.getCameraPosition(), cam.getCameraDirection(), cam.getFov(), shadowCellVisibilityRadius);

		//renderer.renderShadowPoints(view, shadowPointShader, cells);
		std::vector<TreeNode> buds = selTree->AsNodeVector(true);
		renderer.renderShadowsOnBuds(view, shadowPointShader, *world, buds);
		buds = tree2->AsNodeVector(true);
		renderer.renderShadowsOnBuds(view, shadowPointShader, *world, buds);
	}

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

	treeSettingsEdited = false;
	treePreviewChanged = false;
	leafSettingsEdited = false;
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
		if (growTree1)
			generator->growTree(*tree);
		if (growTree2)
			generator->growTree(*tree2);
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