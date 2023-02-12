#include "Renderer.h"
#include <queue>
#include <iostream>
#include "util/util.h"
#include "generation/Branch.h"
#include "types/Buffer.h"
namespace tgen::graphics {
using namespace gl;
using namespace util;
using namespace gen;

constexpr gl::VertexInputBindingDescription cubeDesc[] =
{{.location = 0, .binding = 0,.format = gl::Format::R32G32B32_FLOAT, .offset = 0} };

constexpr gl::VertexInputBindingDescription quadDesc[] =
{{.location = 0, .binding = 0, .format = gl::Format::R32G32_FLOAT, .offset = 0} };

DirectionalLight DirectionalLight::GDirLight = {};

constexpr vec3 lightPos = 0.5f * vec3(-4.0f, 8.0f, 4.0f);
constexpr float lightFarPlane = 10.0f, lightNearPlane = 1.0f;
constexpr vec4 lightOrtho = vec4(-3.0f, 3.0f, -3.0f, 3.0f);
const vec3 lightDir = glm::normalize(vec3(0.4, -0.6, -0.4));
constexpr vec3 lightAmbientCol = 0.5f * vec3(0.2f, 0.2f, 0.15f);
constexpr vec3 lightColor = vec3(1.0f, 1.0f, 1.0f);
constexpr ivec2 shadowSize = ivec2(2048);
void setupDirLight() {
	Texture::TextureCreateData tcd;
	tcd.imageType = ImageType::TEX_2D;
	tcd.mipLevels = 1;
	tcd.size = ivec3(shadowSize, 0);
	tcd.textureFormat = Format::D16_UNORM;
	tcd.minFiltering = Filter::LINEAR;
	tcd.maxFiltering = Filter::LINEAR;
	tcd.wrapping = AddressMode::CLAMP_TO_EDGE;
	DirectionalLight::GDirLight.shadowMap = std::make_shared<Texture>();
	

	auto& dirLight = DirectionalLight::GDirLight;
	dirLight.shadowMap->init(tcd);
	dirLight.ambientCol = lightAmbientCol;
	dirLight.lightColor = lightColor;
	dirLight.lightCam = Camera();
	dirLight.SetDir(lightDir);

	auto& cam = *dirLight.lightCam;
	cam.cameraPosition = lightPos;
	cam.farPlane = lightFarPlane;
	cam.nearPlane = lightNearPlane;
	cam.projection = lightOrtho;
}

void Renderer::init()
{
	glEnable(GL_DEPTH_TEST);
	vec3 cubeVertices[] = {
		// Back face
		{-0.5f, -0.5f, -0.5f}, // Bottom-left
		{ 0.5f,  0.5f, -0.5f}, // top-right
		{ 0.5f, -0.5f, -0.5f}, // bottom-right         
		{ 0.5f,  0.5f, -0.5f}, // top-right
		{-0.5f, -0.5f, -0.5f}, // bottom-left
		{-0.5f,  0.5f, -0.5f}, // top-left
		// Front face		
		{-0.5f, -0.5f,  0.5f}, // bottom-left
		{ 0.5f, -0.5f,  0.5f}, // bottom-right
		{ 0.5f,  0.5f,  0.5f}, // top-right
		{ 0.5f,  0.5f,  0.5f}, // top-right
		{-0.5f,  0.5f,  0.5f}, // top-left
		{-0.5f, -0.5f,  0.5f}, // bottom-left
		// Left face		
		{-0.5f,  0.5f,  0.5f}, // top-right
		{-0.5f,  0.5f, -0.5f}, // top-left
		{-0.5f, -0.5f, -0.5f}, // bottom-left
		{-0.5f, -0.5f, -0.5f}, // bottom-left
		{-0.5f, -0.5f,  0.5f}, // bottom-right
		{-0.5f,  0.5f,  0.5f}, // top-right
		// Right face		
		{ 0.5f,  0.5f,  0.5f}, // top-left
		{ 0.5f, -0.5f, -0.5f}, // bottom-right
		{ 0.5f,  0.5f, -0.5f}, // top-right         
		{ 0.5f, -0.5f, -0.5f}, // bottom-right
		{ 0.5f,  0.5f,  0.5f}, // top-left
		{ 0.5f, -0.5f,  0.5f}, // bottom-left     
		// Bottom face		
		{-0.5f, -0.5f, -0.5f}, // top-right
		{ 0.5f, -0.5f, -0.5f}, // top-left
		{ 0.5f, -0.5f,  0.5f}, // bottom-left
		{ 0.5f, -0.5f,  0.5f}, // bottom-left
		{-0.5f, -0.5f,  0.5f}, // bottom-right
		{-0.5f, -0.5f, -0.5f}, // top-right
		// Top face		 
		{-0.5f,  0.5f, -0.5f}, // top-left
		{ 0.5f,  0.5f,  0.5f}, // bottom-right
		{ 0.5f,  0.5f, -0.5f}, // top-right     
		{ 0.5f,  0.5f,  0.5f}, // bottom-right
		{-0.5f,  0.5f, -0.5f}, // top-left
		{-0.5f,  0.5f,  0.5f}, // bottom-left        
	};

	cubeMesh.vbo.init(cubeVertices);
	
	cubeMesh.inputState = {.vertexBindingDescriptions = cubeDesc};

	vec2 quadVertices[] = {
		// positions
		{-0.5f,  1.0f},
		{-0.5f,  0.0f},
		{ 0.5f,  0.0f},
		
		{ 0.5f,  0.0f},
		{ 0.5f,  1.0f},
		{-0.5f,  1.0f},
	};
	quadMesh.vbo.init(quadVertices);
	quadMesh.inputState = { .vertexBindingDescriptions = quadDesc };


	uint32 pointData[] = { 0 };
	pointMesh.ebo.init(pointData);

	uint32 lineData[] = { 0, 1 };
	lineMesh.ebo.init(lineData);

	camUBO.init(BufferStorageFlag::DYNAMIC_STORAGE);
	lightUBO.init(BufferStorageFlag::DYNAMIC_STORAGE);

	setupDirLight();
}

void Renderer::beginSwapchain()
{
	SwapchainRenderInfo info;
	info.name = "Swapchain";
	info.viewport.size = ivec2(1600, 900);
	BeginSwapchainRendering(info);
	//TODO move to begin swapchain rendering
	glClearColor(0.0f, 0.3f, 0.2f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::endSwapchain()
{
	EndRendering();
}

void Renderer::updateLightUBO(const DrawScene& scene) {
	auto lightUniform = DirLightUniform::fromLightAndVP(scene.light, scene.lightVP);
	lightUBO.bufferData({ &lightUniform, 1 }, 0);
}
void Renderer::updateCameraUBO(const DrawView& view) {
	auto camUniform = CameraUniform::fromCameraAndVP(view.camera, view.VP);
	camUBO.bufferData({ &camUniform, 1 }, 0);
}

void Renderer::startDraw(bool SRGB) {
	if(SRGB)
		glEnable(GL_FRAMEBUFFER_SRGB);
}

void Renderer::endDraw(bool SRGB) {
	if(SRGB)
		glDisable(GL_FRAMEBUFFER_SRGB);
}

void Renderer::renderPlane(const DrawView& view, Shader& shader, mat4 model)
{
	/*vec3 lightDir = glm::normalize(vec3(0.4, -0.6, -0.4));
	vec3 ambientCol = 0.1f * vec3(0.2f, 0.2f, 0.15f);
	vec3 lightColor = vec3(1.0f, 1.0f, 1.0f);
	glDisable(GL_CULL_FACE);
	mat4 vp = view.camera.getProjectionMatrix() * view.camera.getViewMatrix();
	quadVAO->bind();
	shader.bind();
	shader.setUniform("VP", vp);
	shader.setUniform("model", model);

	shader.setUniform("color", vec3(1.0));
	shader.setUniform("camPos", view.camera.getCameraPosition());
	shader.setUniform("lightDir", lightDir);
	shader.setUniform("lightColor", lightColor);
	shader.setUniform("ambientColor", ambientCol);

	glDrawArrays(GL_TRIANGLES, 0, 6);
	glEnable(GL_CULL_FACE);*/
}


/*void Renderer::renderTree(DrawView view, Shader* shader, const TreeNode* root)
{
	mat4 vp = view.camera.getProjectionMatrix() * view.camera.getViewMatrix();

	std::queue< const TreeNode*> queue({ root });
	shader->bind();
	lineVAO.bind();
	shader->setUniform("VP", vp);

	while (!queue.empty()) {
		const TreeNode* sel = queue.front();
		queue.pop();
		if (!sel->bud) {
			queue.push(sel->mainChild);
			queue.push(sel->lateralChild);
			shader->setUniform("pos1", sel->startPos);
			shader->setUniform("pos2", sel->endPos());
			uint32 colorSelected = sel->order;
			shader->setUniform("color", vec3(util::IntNoise2D(colorSelected), util::IntNoise2D(colorSelected, 1), util::IntNoise2D(colorSelected, 2)) * 0.5f + 0.5f);
			glDrawArrays(GL_LINES, 0, 2);
		}
	}
}*/

void RenderShadowPoint(Shader* shader, const vec3& pos, float shadow) {
	/*shader->setUniform("pos", pos);
	glPointSize(shadow * 2.0f);
	shader->setUniform("shadow", shadow);
	glDrawArrays(GL_POINTS, 0, 1);
		*/
}
void Renderer::renderShadowPoints(const DrawView& view, Shader& shader, const std::vector<vec4>& points)
{
	/*glEnable(GL_PROGRAM_POINT_SIZE);
	mat4 vp = view.camera.getProjectionMatrix() * view.camera.getViewMatrix();
	shader.bind();
	pointVAO->bind();
	shader.setUniform("VP", vp);

	uint32 ssbo;
	glCreateBuffers(1, &ssbo);
	glNamedBufferStorage(ssbo, sizeof(vec4) * points.size(), points.data(), GL_MAP_READ_BIT);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);

	glDrawArraysInstanced(GL_POINTS, 0, 1, points.size());

	glDeleteBuffers(1, &ssbo);

	glDisable(GL_PROGRAM_POINT_SIZE);
	*/
}

void Renderer::renderShadowsOnBuds(const DrawView& view, Shader& shader, const TreeWorld& world, const std::vector<TreeNode>& nodes)
{
/*	glEnable(GL_PROGRAM_POINT_SIZE);
	mat4 vp = view.camera.getProjectionMatrix() * view.camera.getViewMatrix();
	shader.bind();
	pointVAO->bind();
	shader.setUniform("VP", vp);
	std::vector<vec4> points;
	for (auto& node : nodes) {
		if (node.nodeStatus == TreeNode::BUD) {
			ivec3 cell = world.coordinateToCell(node.startPos);
			for (int i = 0; i < 10; i++) {
				if (cell.y + i >= world.worldSize.y)
					break;
				ShadowCell shadowCell = world.getCellAt(cell + ivec3(0, i, 0));
				points.emplace_back(vec4(node.startPos + vec3(0, world.cellSize * float(i), 0), shadowCell.shadow));
			}
		}
	}

	uint32 ssbo;
	glCreateBuffers(1, &ssbo);
	glNamedBufferStorage(ssbo, sizeof(vec4) * points.size(), points.data(), GL_MAP_READ_BIT);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);

	glDrawArraysInstanced(GL_POINTS, 0, 1, points.size());

	glDeleteBuffers(1, &ssbo);
	glDisable(GL_PROGRAM_POINT_SIZE);
	*/
}

void Renderer::renderBBoxLines(const DrawView& view, Shader& shader, const BBox& bbox, const vec3& color)
{
	GraphicsPipeline pipeline("BBox Lines", shader);
	pipeline.inputAssemblyState.topology = PrimitiveTopology::LINE_LIST;

	Cmd::ScopedGraphicsPipeline _(pipeline);
	
	Cmd::util::BindMesh(lineMesh);
	Cmd::SetUniform("VP", view.VP);
	Cmd::SetUniform("color", color);

	auto const drawLine = [&](const vec3& first, const vec3& second) {
		Cmd::SetUniform("pos1", first);
		Cmd::SetUniform("pos2", second);
		Cmd::DrawIndexed(2);
	};

	vec3 first = bbox.min;
	vec3 second = bbox.min;

	second.x = bbox.max.x;
	second.z = bbox.min.z;

	drawLine(first, second);

	second.x = bbox.min.x;
	second.z = bbox.max.z;

	drawLine(first, second);

	first.x = bbox.max.x;
	first.z = bbox.max.z;

	second.x = bbox.min.x;
	second.z = bbox.max.z;

	drawLine(first, second);

	second.x = bbox.max.x;
	second.z = bbox.min.z;

	drawLine(first, second);
	first = bbox.min;
	first.y = bbox.max.y;
	second.y = bbox.max.y;

	second.x = bbox.max.x;
	second.z = bbox.min.z;

	drawLine(first, second);

	second.x = bbox.min.x;
	second.z = bbox.max.z;

	drawLine(first, second);

	first.x = bbox.max.x;
	first.z = bbox.max.z;

	second.x = bbox.min.x;
	second.z = bbox.max.z;

	drawLine(first, second);

	second.x = bbox.max.x;
	second.z = bbox.min.z;

	drawLine(first, second);

	first = bbox.min;
	second = bbox.min;
	second.y = bbox.max.y;

	drawLine(first, second);

	first.x = bbox.max.x;
	second.x = bbox.max.x;
	drawLine(first, second);

	first.z = bbox.max.z;
	second.z = bbox.max.z;
	drawLine(first, second);

	first.x = bbox.min.x;
	second.x = bbox.min.x;
	drawLine(first, second);
}

void Renderer::setupSkybox(const rc<CubemapTexture>& skyboxTexture, const rc<Shader>& skyboxShader)
{
	this->skyboxShader = skyboxShader;
	this->skyboxTexture = skyboxTexture;
}

void Renderer::renderSkybox(const DrawView& view)
{
	static GraphicsPipeline pipeline = []() ->GraphicsPipeline{
		GraphicsPipeline pipeline("Skybox Pipeline", *Renderer::getRenderer().skyboxShader);
		pipeline.rasterizationState.cullMode = CullMode::NONE;
		pipeline.depthState.depthCompareOp = CompareOp::LESS_OR_EQUAL;
		pipeline.vertexInputState = Renderer::getRenderer().getCubeMesh().inputState;
		return pipeline;
	}();
	Cmd::ScopedGraphicsPipeline _(pipeline);
	glBindTextureUnit(skyboxShader->getTextureIndex("skybox"), skyboxTexture->getHandle());
	mat4 vp = view.camera.getProjectionMatrix() * mat4(mat3(view.camera.getViewMatrix()));
	Cmd::SetUniform("VP", vp);
	Cmd::util::BindMesh(cubeMesh);
	Cmd::Draw(36);
}

void Renderer::renderTest(const DrawView& view) {
	
	static GraphicsPipeline pipeline = []() -> GraphicsPipeline {
		GraphicsPipeline pipeline("Test Pipeline", *Renderer::getRenderer().pointShader);
		pipeline.inputAssemblyState.topology = PrimitiveTopology::POINT_LIST;
		pipeline.rasterizationState.pointSize = 10.0f;
		return pipeline;
	}();
	Cmd::ScopedGraphicsPipeline _(pipeline);
	Cmd::SetUniform("VP", view.VP);
	Cmd::SetUniform("pos", vec3(-0.0f, 0.0f, 0.0f));
	Cmd::util::BindMesh(pointMesh);
	Cmd::DrawIndexed(1);
}


void Renderer::startShadowPass()
{
	static gl::RenderDepthStencilAttachment attachment = 
	{.texture = DirectionalLight::GDirLight.shadowMap, 
		.clearOnLoad = true, 
		.clearValue = 1.0f, };
	static RenderInfo renderInfo {.name = "Shadow pass", .depthAttachment = &attachment};

	BeginRendering(renderInfo);
}

void Renderer::endShadowPass()
{
	EndRendering();
}

const ArrayMesh<vec3>& Renderer::getCubeMesh()
{
	return cubeMesh;
}

const ArrayMesh<vec2>& Renderer::getQuadMesh()
{
	return quadMesh;
}

const IndexedMesh<IndexType::UNSIGNED_INT>& Renderer::getLineMesh()
{
	return lineMesh;
}

const IndexedMesh<IndexType::UNSIGNED_INT>& Renderer::getPointMesh()
{
	return pointMesh;
}
const gl::UBO<CameraUniform>& Renderer::getCamUBO() {
	return camUBO;
}
const gl::UBO<DirLightUniform>& Renderer::getLightUBO() {
	return lightUBO;
}

/*void Renderer::render(const DrawView view, DrawCall* drawCalls, uint32 count)
{
	glClearColor(0.0f, 0.3f, 0.2f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	mat4 vp = view.camera.getProjectionMatrix() * view.camera.getViewMatrix();
	for (uint32 i = 0; i < count; i++) {
		DrawCall& call = drawCalls[i];
		RenderMaterial& curMaterial = call.material;
		if (&curMaterial != m_LastMaterial) {
			changeToMaterial(curMaterial);
		}
		Shader& cur = *m_LastShader;
		cur.setUniform("MVP", call.transform.getModelMatrix());
		glBindVertexArray(call.vao);
		glDrawElements(GL_TRIANGLES, call.indexCount, GL_UNSIGNED_INT, 0);
	}
}

void Renderer::changeToMaterial(RenderMaterial& material)
{
	m_LastMaterial = &material;
	if (material.m_Shader != m_LastShader) {
		material.m_Shader->bind();
		m_LastShader = material.m_Shader;
	}
}
*/
}