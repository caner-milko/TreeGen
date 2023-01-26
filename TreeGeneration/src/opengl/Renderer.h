#pragma once
#include <vector>
#include "./Camera.h"
#include "../Definitions.h"
#include "./DrawView.h"
#include "./Shader.h"
#include "./GLVertexArray.h"
#include "../TreeNode.h"
#include "../TreeWorld.h"
#include "CubemapTexture.h"
#include "FrameBuffer.h"
#include <glad/glad.h>
class Renderer {

public:
	void init();
	void startFrame();
	void endFrame();
	void startDraw(bool SRGB);
	void endDraw(bool SRGB);
	void renderPlane(DrawView view, Shader& shader, mat4 model);

	void renderShadowPoints(DrawView view, Shader& shader, const std::vector<vec4>& points);

	void renderShadowsOnBuds(DrawView view, Shader& shader, const TreeWorld& world, const std::vector<TreeNode>& buds);

	void renderBBoxLines(DrawView view, Shader& shader, const BBox& bbox, const vec3& color);

	void setupSkybox(const sp<CubemapTexture>& skyboxTexture, const sp<Shader>& skyboxShader);

	void renderSkybox(DrawView view);

	void startShadowPass();
	void endShadowPass(DrawView view);

	const sp<GLVertexArray>& getCubeVAO();
	const sp<GLVertexArray>&  getQuadVAO();
	const sp<GLVertexArray>& getLineVAO();
	const sp<GLVertexArray>& getPointVAO();

	void bindFramebuffer(const sp<FrameBuffer>& fb, GLenum bindType = GL_FRAMEBUFFER);


	sp<FrameBuffer> shadowFrameBuffer;
	sp<Texture> shadowTexture;
	sp<Shader> pointShader;
private:


	sp<CubemapTexture> skyboxTexture = nullptr;
	sp<Shader> skyboxShader = nullptr;


	sp<Shader> lastShader = nullptr;

	sp<GLVertexArray> cubeVAO, quadVAO, lineVAO, pointVAO;
};
