#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "./TreeGenerator.h"
#include "./TreeWorld.h"
#include <random>
#include <time.h>

#include "./opengl/Renderer.h"
#include "./opengl/ResourceManager.h"

float deltaTime = 0.0f;	// Time between current frame and last frame
float currentFrame = 0.0f;
float lastFrame = 0.0f; // Time of last frame

float yaw = -90.0f, pitch = 0.0f;
float lastX = 400, lastY = 300;
bool firstMouse = true;
float fov = 45.0f;

bool fPressed = false;

void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

Camera cam(1.0f);

std::unique_ptr<TreeWorld> world;
std::unique_ptr<TreeGenerator> generator;
Tree* tree;

int main() {
	srand(time(NULL));
	cam.setCameraPosition({ -1.0f, 0.0f, 0.0f });
	generator = std::make_unique<TreeGenerator>();

	world = std::make_unique<TreeWorld>(50, 100, 50, vec3(-12.5f, 0.0f, -12.5f), 0.5f);

	tree = &generator->createTree(*world, vec3(0.0f));

	/*std::cout << "---------------------------" << std::endl;
	tree.printTreeRecursive(*tree.root, "");
	std::cout << "---------------------------" << std::endl;
	generator.growTree(tree);
	std::cout << "---------------------------" << std::endl;
	tree.printTreeRecursive(*tree.root, "");
	std::cout << "---------------------------" << std::endl;
	generator.growTree(tree);
	std::cout << "---------------------------" << std::endl;
	tree.printTreeRecursive(*tree.root, "");
	std::cout << "---------------------------" << std::endl;*/

	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow((int)600, (int)600, "TreeGen", NULL, NULL);

	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return 0;
	}
	glfwMakeContextCurrent(window);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return 0;
	}

	Renderer renderer;
	renderer.init();

	Shader* shader = ResourceManager::getInstance().loadShader("line_shader", "./Assets/Shaders/line_vert.glsl", "./Assets/Shaders/line_frag.glsl");
	while (!glfwWindowShouldClose(window)) {
		
		currentFrame = (float)glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		processInput(window);
		renderer.renderTree({ cam }, shader, tree->root.get());
		glfwSwapBuffers(window);
		glfwPollEvents();
		lastFrame = currentFrame;
	}
}


void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	vec3 camPos = cam.getCameraPosition();
	const float cameraSpeed = 2.5f * deltaTime;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camPos += cameraSpeed * cam.getCameraDirection();
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camPos -= cameraSpeed * cam.getCameraDirection();
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camPos -= cam.getCameraRight() * cameraSpeed;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camPos += cam.getCameraRight() * cameraSpeed;
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		camPos += glm::vec3(0.0f, cameraSpeed, 0.0f);
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
		camPos += glm::vec3(0.0f, -cameraSpeed, 0.0f);
	if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
		if(!fPressed) {
			generator->growTree(*tree);
			fPressed = true;
		}
	}
	else {
		fPressed = false;
	}

	cam.setCameraPosition(camPos);

}

void mouse_callback(GLFWwindow *window, double xpos, double ypos)
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

	cam.setYaw(cam.getYaw() + xoffset);
	cam.setPitch(cam.getPitch() + yoffset);
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
	cam.setFov(cam.getFov() - (float)yoffset);
}
