#pragma once
#include "types/Texture.h"
#include "Pipeline.h"
#include "types/Sampler.h"
#include "types/Buffer.h"
#include "types/VertexArray.h"
#include "types/FrameBuffer.h"
#include <span>
#include <optional>
#include <variant>
namespace tgen::graphics::gl 
{
struct ClearDepthStencilValue
{
	float depth = 1.0f;
	int32 stencil = 0;
};
struct RenderColorAttachment
{
	rc<Texture> texture;
	bool clearOnLoad = true;
	std::variant<vec4, ivec4, uvec4> clearValue = vec4(0.0f);
};
struct RenderDepthStencilAttachment
{
	rc<Texture> texture;
	bool clearOnLoad = true;
	ClearDepthStencilValue clearValue{};
};
struct Viewport {
	ivec2 offset{};
	ivec2 size{};
};
struct RenderInfo {
	std::string name;
	std::optional<Viewport> viewport = std::nullopt;
	std::span<RenderColorAttachment> colorAttachments;
	RenderDepthStencilAttachment* depthAttachment{};
	RenderDepthStencilAttachment* stencilAttachment{};
};
struct SwapchainRenderInfo {
	std::string name;
	Viewport viewport;
};

struct RenderingContext {
	bool isPipelineDebugGroupPushed = false;
	bool isRendering = false;

	bool isRenderingToSwapchain = false;

	bool isPipelineBound = false;
	std::optional<GraphicsPipeline> lastBoundPipeline = std::nullopt;

	Viewport viewport{};

	std::optional<VertexArray> currentVao = std::nullopt;
	std::optional<FrameBuffer> currentFbo = std::nullopt;

	PrimitiveTopology currentTopology{};
	bool isIndexBufferBound = false;
	IndexType currentIndexType{};
};
extern RenderingContext context;

extern void BeginSwapchainRendering(const SwapchainRenderInfo& renderInfo);
extern void BeginRendering(const RenderInfo& renderInfo);
extern void EndRendering();
namespace Cmd {
	extern void Draw(uint32_t vertexCount,
        uint32_t instanceCount = 1,
        uint32_t firstVertex = 0,
        uint32_t firstInstance = 0);

	extern void DrawIndexed(uint32_t indexCount,
        uint32_t instanceCount = 1,
        uint32_t firstIndex = 0,
        int32_t vertexOffset = 0,
        uint32_t firstInstance = 0);

	extern void SetViewport(const Viewport& viewport);

	extern void BindGraphicsPipeline(const GraphicsPipeline& pipeline);
	extern void UnbindGraphicsPipeline();

struct ScopedGraphicsPipeline {

	ScopedGraphicsPipeline(const GraphicsPipeline& pipeline) {
		assert(!context.isPipelineBound);
		BindGraphicsPipeline(pipeline);
		isValid = true;
	}
	~ScopedGraphicsPipeline() {
		if (isValid) {
			UnbindGraphicsPipeline();
		}
	}
	ScopedGraphicsPipeline(ScopedGraphicsPipeline&& other) noexcept : isValid(std::exchange(other.isValid, false)) {
	}
	MOVE_OPERATOR(ScopedGraphicsPipeline);
	DELETE_COPY_CONSTRUCTORS(ScopedGraphicsPipeline);
private:
	bool isValid = false;
};

	extern void BindVBO(uint32 index, const gl::Buffer<BufferType::VBO>& buffer, uint64 offset, uint64 stride);
	extern void BindEBO(const gl::Buffer<BufferType::EBO>& buffer, IndexType indexType);
	extern void BindSSBO(uint32 index, const gl::Buffer<BufferType::SSBO>& buffer, uint64 offset, uint64 size);
	extern void BindUBO(uint32 index, const gl::Buffer<BufferType::UBO>& buffer, uint64 offset, uint64 size);
	extern void BindSampledImage(uint32 index, const Texture& texture, const Sampler& sampler);

	extern Shader& GetShader();
	template<typename T>
	static void SetUniform(std::string_view uniformName, const T& val) {
		assert(context.isRendering);
		GetShader().setUniform(uniformName, val);
	}
	template<typename T>
	static void SetUniform(uint32 uniformLoc, const T& val) {
		assert(context.isRendering);
		GetShader().setUniform(uniformLoc, val);
	}


}
}