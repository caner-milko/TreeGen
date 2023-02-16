#pragma once
#include <vector>
#include "Camera.h"
#include "Common.h"
#include "DrawView.h"
#include "opengl/types/Shader.h"
#include "opengl/types/VertexArray.h"
#include "generation/TreeNode.h"
#include "generation/TreeWorld.h"
#include "opengl/types/CubemapTexture.h"
#include "opengl/types/FrameBuffer.h"
#include <glad/glad.h>
#include "Mesh.h"
#include "DrawScene.h"
namespace tgen::graphics {
class Renderer {

public:



	static Renderer& getRenderer() {

		static Renderer* renderer = []() -> Renderer* {
			Renderer* rend = new Renderer();
			rend->init();
			return rend;
		}();

		return *renderer;
	}

	void beginSwapchain();
	void endSwapchain();

	void updateLightUBO(const DrawScene& scene);
	void updateCameraUBO(const DrawView& view);

	void startDraw(bool SRGB);
	void endDraw(bool SRGB);
	void renderPlane(const DrawView& view, gl::Shader& shader, mat4 model);

	void renderShadowPoints(const DrawView& view, gl::Shader& shader, const std::vector<vec4>& points);

	void renderShadowsOnBuds(const DrawView& view, gl::Shader& shader, const gen::TreeWorld& world, const std::vector<gen::TreeNode>& buds);

	void renderBBoxLines(const DrawView& view, gl::Shader& shader, const util::BBox& bbox, const vec3& color);

	void setupSkybox(const rc<gl::CubemapTexture>& skyboxTexture, const rc<gl::Shader>& skyboxShader);

	void renderSkybox(const DrawView& view);

	void renderTest(const DrawView& view);

	void startShadowPass();
	void endShadowPass();

	const ArrayMesh<vec3>& getCubeMesh();
	const ArrayMesh<vec2>& getQuadMesh();
	const IndexedMesh<gl::IndexType::UNSIGNED_INT>& getLineMesh();
	const IndexedMesh<gl::IndexType::UNSIGNED_INT>& getPointMesh();

	const gl::UBO<CameraUniform>& getCamUBO();
	const gl::UBO<DirLightUniform>& getLightUBO();

	rc<gl::Shader> pointShader;
private:

	Renderer() = default;
	~Renderer() = default;
	Renderer(Renderer&&) = delete;
	DELETE_COPY_CONSTRUCTORS(Renderer)
		void init();

	rc<gl::CubemapTexture> skyboxTexture = nullptr;
	rc<gl::Shader> skyboxShader = nullptr;

	gl::UBO<CameraUniform> camUBO;
	gl::UBO<DirLightUniform> lightUBO;

	ArrayMesh<vec3> cubeMesh;
	ArrayMesh<vec2> quadMesh;
	IndexedMesh<gl::IndexType::UNSIGNED_INT> lineMesh, pointMesh;
};
}