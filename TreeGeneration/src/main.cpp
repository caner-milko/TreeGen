#include <iostream>
#include <glad/glad.h>

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include <stdio.h>

#include <GLFW/glfw3.h>
#include "./TreeGenerator.h"
#include "./TreeWorld.h"
#include <time.h>
#include "./opengl/Renderer.h"
#include "./opengl/ResourceManager.h"

#include "./Branch.h"

int width = 1920;
int height = 1080;

float deltaTime = 0.0f;	// Time between current frame and last frame
float currentFrame = 0.0f;
float lastFrame = 0.0f; // Time of last frame

float yaw = -90.0f, pitch = 0.0f;
float lastX = width / 2, lastY = height / 2;
bool firstMouse = true;
float fov = 45.0f;

bool cursorDisabled = true;

void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

Camera cam(width / (float)height);

std::unique_ptr<TreeWorld> world;
std::unique_ptr<TreeGenerator> generator;
Tree* tree;
Tree* tree2;

bool growTree1 = true;
bool growTree2 = true;
int main() {
	cam.setCameraPosition({ -1.0f, 0.0f, 0.0f });
	generator = std::make_unique<TreeGenerator>();

	float baseLen = .4f;

	float worldSizeX = 25.0;
	float worldSizeY = 50.0;
	float worldSizeZ = 25.0;

	world = std::make_unique<TreeWorld>(worldSizeX / baseLen * 2.0f, worldSizeY / baseLen * 2.0f, worldSizeZ / baseLen * 2.0f, vec3(-12.5f, 0.0f, -12.5f), baseLen / 2.0f);

	tree = generator->createTree(*world, vec3(0.0f));
	tree2 = generator->createTree(*world, vec3(2.0f, 0.0f, 1.0f));

	tree->growthData.baseLength = baseLen;
	tree2->growthData.baseLength = baseLen;

	tree->init();
	tree2->init();

	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(width, height, "TreeGen", NULL, NULL);

	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return 0;
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return 0;
	}



	const char* glsl_version = "#version 130";

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

	Renderer renderer;
	renderer.init();

	Shader* treeLineShader = ResourceManager::getInstance().loadShader("line_shader", "./Assets/Shaders/line_vert.glsl", "./Assets/Shaders/line_frag.glsl");

	Shader* shadowPointShader = ResourceManager::getInstance().loadShader("shadowPoint_shader", "./Assets/Shaders/shadowPoint_vert.glsl", "./Assets/Shaders/shadowPoint_frag.glsl");

	Shader* treeQuadShader = ResourceManager::getInstance().loadShader("treeQuad_shader", "./Assets/Shaders/treeQuad_vert.glsl", "./Assets/Shaders/treeQuad_frag.glsl");
	Shader* treeQuadMarchShader = ResourceManager::getInstance().loadShader("treeQuadMarch_shader", "./Assets/Shaders/treeQuad_vert.glsl", "./Assets/Shaders/treeQuadMarch_frag.glsl");

	Texture* tex = ResourceManager::getInstance().loadTexture("test", "./Assets/Textures/bark.jpg");

	float shadowCellVisibilityRadius = 10.0f;
	bool showShadowGrid = false;
	bool renderPreviewTree = false;
	std::unique_ptr<Tree> previewTree;

	while (!glfwWindowShouldClose(window)) {
		renderer.startFrame();
		glfwPollEvents();

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		/*
		// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
		if (show_demo_window)
			ImGui::ShowDemoWindow(&show_demo_window);

		// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
		{
			static int counter = 0;

			ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

			ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
			ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
			ImGui::Checkbox("Another Window", &show_another_window);

			ImGui::SliderFloat("Apical Control", &tree->growthData.apicalControl, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
			ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

			if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
				counter++;
			ImGui::SameLine();
			ImGui::Text("counter = %d", counter);

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::End();
		}*/

		bool treeSettingsEdited = false;
		bool treePreviewChanged = false;
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

				treeSettingsEdited = ImGui::SliderFloat("Apical Control", &growthData.apicalControl, 0.0f, 1.0f);
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
				tree2->growthData = growthData;



			}

			if (ImGui::CollapsingHeader("Render Options")) {

				treeSettingsEdited |= treePreviewChanged = ImGui::Checkbox("Render Tree Preview", &renderPreviewTree);
				ImGui::Checkbox("Show Shadow Grid", &showShadowGrid);
				ImGui::SliderFloat("Shadow Cell Visibility Radius", &shadowCellVisibilityRadius, 0.5f, 20.0f);
			}
			ImGui::Checkbox("Grow Tree 1", &growTree1);
			ImGui::Checkbox("Grow Tree 2", &growTree2);
			ImGui::End();
		}

		if (treePreviewChanged && !renderPreviewTree) {
			previewTree.reset();
		}

		if (renderPreviewTree) {
			if (treeSettingsEdited || (previewTree != nullptr && previewTree->age != tree->age + 1)) {
				previewTree = std::make_unique<Tree>(*tree);
				generator->growTree(*previewTree);
			}
		}


		currentFrame = (float)glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		processInput(window);

		DrawView view{ cam };

		Tree* selTree = renderPreviewTree ? previewTree.get() : tree;

		auto nodes = selTree->AsBranchVector();

		renderer.renderTree2(view, treeQuadMarchShader, nodes);

		nodes = tree2->AsBranchVector();
		renderer.renderTree2(view, treeQuadMarchShader, nodes);

		if (showShadowGrid) {
			world->calculateShadows();
			//std::vector<std::tuple<vec3, float>> cells = world->renderShadowCells(cam.getCameraPosition(), cam.getCameraDirection(), cam.getFov(), shadowCellVisibilityRadius);

			//renderer.renderShadowPoints(view, shadowPointShader, cells);
			std::vector<TreeNode> buds = selTree->AsNodeVector(true);
			renderer.renderShadowsOnBuds(view, shadowPointShader, *world, buds);
			buds = tree2->AsNodeVector(true);
			renderer.renderShadowsOnBuds(view, shadowPointShader, *world, buds);
		}


		lastFrame = currentFrame;


		// Rendering
		ImGui::Render();
		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// Update and Render additional Platform Windows
		// (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
		//  For this specific demo app we could also call glfwMakeContextCurrent(window) directly)
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}

		glfwSwapBuffers(window);
		renderer.endFrame();
	}
}


void processInput(GLFWwindow* window)
{
	if (ImGui::IsKeyDown(ImGuiKey_Escape))
		glfwSetWindowShouldClose(window, true);
	vec3 camPos = cam.getCameraPosition();
	const float cameraSpeed = 2.5f * deltaTime;
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

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse) // initially set to true
	{
		lastX = (float)xpos;
		lastY = (float)ypos;
		firstMouse = false;
	}
	float xoffset = (float)xpos - lastX;
	float yoffset = lastY - (float)ypos; // reversed since y-coordinates range from bottom to top
	lastX = (float)xpos;
	lastY = (float)ypos;

	const float sensitivity = 0.1f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	if (!cursorDisabled)
		return;
	cam.setYaw(cam.getYaw() + xoffset);
	cam.setPitch(cam.getPitch() + yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	cam.setFov(cam.getFov() - (float)yoffset);
}
