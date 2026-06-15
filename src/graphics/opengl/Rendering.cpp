#include "Rendering.h"
#include "ApiToEnum.h"
#include <glm/gtc/type_ptr.hpp>
namespace tgen::graphics::gl {
using namespace tgen::graphics::gl::detail;

RenderingContext context = {};

static void GLEnableOrDisable(GLenum state, bool val) {
	if (val)
		glEnable(state);
	else
		glDisable(state);
}

static size_t GetIndexSize(IndexType indexType)
{
	switch (indexType)
	{
	case IndexType::UNSIGNED_BYTE: return 1;
	case IndexType::UNSIGNED_SHORT: return 2;
	case IndexType::UNSIGNED_INT: return 4;
	default: assert(false); return 0;
	}
}

void BeginSwapchainRendering(const SwapchainRenderInfo& renderInfo)
{
	context.isRendering = true;
	context.isRenderingToSwapchain = true;
	context.currentFbo = std::nullopt;
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, static_cast<GLsizei>(renderInfo.name.size()), renderInfo.name.data());
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//TODO clear on load
	//TODO srgb
	Cmd::SetViewport(renderInfo.viewport);
}

FrameBuffer CreateFrameBuffer(const RenderInfo& renderInfo) {
	std::string_view msg = "Create Pipeline Framebuffer";
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, msg.size(), msg.data());
	FrameBuffer fbo;
	if (!fbo.init()) assert(false);
	int i = 0;

	ivec2 minSize;
	for (const auto& colorAttachment : renderInfo.colorAttachments)
	{
		minSize = glm::min(ivec2(colorAttachment.texture->getCreateData().size), minSize);
		fbo.attach(GL_COLOR_ATTACHMENT0 + i, *colorAttachment.texture);
		i++;
	}
	if (renderInfo.depthAttachment && renderInfo.stencilAttachment && renderInfo.depthAttachment == renderInfo.stencilAttachment) {
		minSize = glm::min(ivec2(renderInfo.depthAttachment->texture->getCreateData().size), minSize);
		fbo.attach(GL_DEPTH_STENCIL_ATTACHMENT, *renderInfo.depthAttachment->texture);
	}
	else if (renderInfo.depthAttachment) {
		minSize = glm::min(ivec2(renderInfo.depthAttachment->texture->getCreateData().size), minSize);
		fbo.attach(GL_DEPTH_ATTACHMENT, *renderInfo.depthAttachment->texture);
	}
	else if (renderInfo.stencilAttachment) {
		minSize = glm::min(ivec2(renderInfo.stencilAttachment->texture->getCreateData().size), minSize);
		fbo.attach(GL_STENCIL_ATTACHMENT, *renderInfo.stencilAttachment->texture);
	}
	glPopDebugGroup();
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		assert(false);
	return fbo;
}

void BeginRendering(const RenderInfo& renderInfo)
{
	if (context.isRendering)
		glPopDebugGroup();
	context.isRendering = true;
	context.isRenderingToSwapchain = false;
	//TODO add framebuffer cache
	context.currentFbo = CreateFrameBuffer(renderInfo);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		assert(false);
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, static_cast<GLsizei>(renderInfo.name.size()), renderInfo.name.data());

	ivec2 minSize = ivec2(std::numeric_limits<int>::max(), std::numeric_limits<int>::max());
	int i = 0;
	for (const auto& colorAttachment : renderInfo.colorAttachments)
	{
		minSize = glm::min(ivec2(colorAttachment.texture->getCreateData().size), minSize);
		if (colorAttachment.clearOnLoad) {
			glColorMaski(i, true, true, true, true);

			auto format = colorAttachment.texture->getCreateData().textureFormat;
			auto baseTypeClass = detail::FormatToBaseTypeClass(format);

			auto& ccv = colorAttachment.clearValue;

			switch (baseTypeClass)
			{
			case detail::GlBaseTypeClass::FLOAT:
				assert((std::holds_alternative<vec4>(ccv)));
				glClearNamedFramebufferfv(context.currentFbo->getHandle(),
					GL_COLOR, i, glm::value_ptr(std::get<vec4>(ccv)));
				break;
			case detail::GlBaseTypeClass::SINT:
				assert((std::holds_alternative<ivec4>(ccv)));
				glClearNamedFramebufferiv(context.currentFbo->getHandle(),
					GL_COLOR, i, glm::value_ptr(std::get<ivec4>(ccv)));
				break;
			case detail::GlBaseTypeClass::UINT:
				assert((std::holds_alternative<uvec4>(ccv)));
				glClearNamedFramebufferuiv(context.currentFbo->getHandle(),
					GL_COLOR, i, glm::value_ptr(std::get<uvec4>(ccv)));
				break;
			default: assert(false);
			}
		}
		i++;
	}

	{
		if (renderInfo.depthAttachment && renderInfo.depthAttachment->clearOnLoad && renderInfo.stencilAttachment && renderInfo.stencilAttachment->clearOnLoad)
		{
			// clear depth and stencil simultaneously
			glDepthMask(true);
			glStencilMask(true);

			auto& clearDepth = renderInfo.depthAttachment->clearValue;
			auto& clearStencil = renderInfo.stencilAttachment->clearValue;

			glClearNamedFramebufferfi(context.currentFbo->getHandle(), GL_DEPTH_STENCIL, 0, clearDepth.depth, clearStencil.stencil);
		}
		else if ((renderInfo.depthAttachment && renderInfo.depthAttachment->clearOnLoad) &&
			(!renderInfo.stencilAttachment || !renderInfo.stencilAttachment->clearOnLoad))
		{
			// clear just depth
			glDepthMask(true);

			glClearNamedFramebufferfv(context.currentFbo->getHandle(), GL_DEPTH, 0, &renderInfo.depthAttachment->clearValue.depth);
		}
		else if ((renderInfo.stencilAttachment && renderInfo.stencilAttachment->clearOnLoad) &&
			(!renderInfo.depthAttachment || !renderInfo.depthAttachment->clearOnLoad))
		{
			// clear just stencil
			glStencilMask(true);

			glClearNamedFramebufferiv(context.currentFbo->getHandle(), GL_STENCIL, 0, &renderInfo.stencilAttachment->clearValue.stencil);
		}

		if (renderInfo.depthAttachment)
			minSize = glm::min(minSize, ivec2(renderInfo.depthAttachment->texture->getCreateData().size));
		if (renderInfo.stencilAttachment)
			minSize = glm::min(minSize, ivec2(renderInfo.stencilAttachment->texture->getCreateData().size));
	}
	Viewport viewport;
	if (renderInfo.viewport)
		viewport = *renderInfo.viewport;
	else
		viewport = { .offset = ivec2(0), .size = minSize };

	Cmd::SetViewport(viewport);
	glBindFramebuffer(GL_FRAMEBUFFER, context.currentFbo->getHandle());

}

void EndRendering()
{
	context.isRendering = false;
	context.isRenderingToSwapchain = false;
	context.currentFbo = std::nullopt;
	if (context.isPipelineDebugGroupPushed) {
		context.isPipelineDebugGroupPushed = false;
		glPopDebugGroup();
	}
	glPopDebugGroup();
}
namespace Cmd {
void Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{
	assert(context.isRendering && context.isPipelineBound);

	glDrawArraysInstancedBaseInstance(detail::PrimitiveTopologyToGL(context.currentTopology),
		firstVertex,
		vertexCount,
		instanceCount,
		firstInstance);

}

void DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance)
{
	assert(context.isRendering && context.isPipelineBound);
	glDrawElementsInstancedBaseVertexBaseInstance(
		detail::PrimitiveTopologyToGL(context.currentTopology),
		indexCount,
		detail::IndexTypeToGL(context.currentIndexType),
		reinterpret_cast<void*>(static_cast<uintptr_t>(firstIndex * GetIndexSize(context.currentIndexType))),
		instanceCount,
		vertexOffset,
		firstInstance);
}

void SetViewport(const Viewport& viewport) {
	assert(context.isRendering);
	glViewport(viewport.offset.x, viewport.offset.y, viewport.size.x, viewport.size.y);
	context.viewport = viewport;
}

VertexArray CreateVAO(const GraphicsPipeline& pipeline) {
	std::string_view msg = "Create Vertex Array for Pipeline";
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, msg.size(), msg.data());
	VertexArray VAO;
	VAO.init();

	auto& inputState = pipeline.vertexInputState;
	for (uint32_t i = 0; i < inputState.vertexBindingDescriptions.size(); i++)
	{
		const auto& desc = inputState.vertexBindingDescriptions[i];
		VAO.enableAttribute(desc.location, desc.binding, desc.format, desc.offset);
	}
	glPopDebugGroup();
	return VAO;
}

void UnbindGraphicsPipeline() {
	context.isPipelineBound = false;
}

void BindGraphicsPipeline(const GraphicsPipeline& pipeline)
{
	assert(context.isRendering && pipeline.shader);
	if (context.isPipelineDebugGroupPushed)
	{
		context.isPipelineDebugGroupPushed = false;
		glPopDebugGroup();
	}
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION,
		0,
		static_cast<GLsizei>(pipeline.name.size()),
		pipeline.name.data());
	context.isPipelineDebugGroupPushed = true;

	if (context.lastBoundPipeline) {
		glEnable(GL_FRAMEBUFFER_SRGB);
	}

	glUseProgram(pipeline.shader->getHandle());

	const auto& ias = pipeline.inputAssemblyState;
	if (!context.lastBoundPipeline ||
		ias.primitiveRestartEnable != context.lastBoundPipeline->inputAssemblyState.primitiveRestartEnable)
	{
		GLEnableOrDisable(GL_PRIMITIVE_RESTART_FIXED_INDEX, ias.primitiveRestartEnable);
	}
	context.currentTopology = ias.topology;

	context.currentVao = CreateVAO(pipeline);
	glBindVertexArray(context.currentVao->getHandle());

	//////////////////////////////////////////////////////////////// rasterization
	const auto& rs = pipeline.rasterizationState;
	if (!context.lastBoundPipeline ||
		rs.depthClampEnable != context.lastBoundPipeline->rasterizationState.depthClampEnable)
	{
		GLEnableOrDisable(GL_DEPTH_CLAMP, rs.depthClampEnable);
	}

	if (!context.lastBoundPipeline || rs.polygonMode != context.lastBoundPipeline->rasterizationState.polygonMode)
	{
		glPolygonMode(GL_FRONT_AND_BACK, PolygonModeToGL(rs.polygonMode));
	}

	if (!context.lastBoundPipeline || rs.cullMode != context.lastBoundPipeline->rasterizationState.cullMode)
	{
		GLEnableOrDisable(GL_CULL_FACE, rs.cullMode != CullMode::NONE);
		if (rs.cullMode != CullMode::NONE)
		{
			glCullFace(CullModeToGL(rs.cullMode));
		}
	}

	if (!context.lastBoundPipeline || rs.frontFace != context.lastBoundPipeline->rasterizationState.frontFace)
	{
		glFrontFace(FrontFaceToGL(rs.frontFace));
	}

	if (!context.lastBoundPipeline ||
		rs.depthBiasEnable != context.lastBoundPipeline->rasterizationState.depthBiasEnable)
	{
		GLEnableOrDisable(GL_POLYGON_OFFSET_FILL, rs.depthBiasEnable);
		GLEnableOrDisable(GL_POLYGON_OFFSET_LINE, rs.depthBiasEnable);
		GLEnableOrDisable(GL_POLYGON_OFFSET_POINT, rs.depthBiasEnable);
	}

	if (!context.lastBoundPipeline ||
		rs.depthBiasSlopeFactor != context.lastBoundPipeline->rasterizationState.depthBiasSlopeFactor ||
		rs.depthBiasConstantFactor != context.lastBoundPipeline->rasterizationState.depthBiasConstantFactor)
	{
		glPolygonOffset(rs.depthBiasSlopeFactor, rs.depthBiasConstantFactor);
	}

	if (!context.lastBoundPipeline || rs.lineWidth != context.lastBoundPipeline->rasterizationState.lineWidth)
	{
		glLineWidth(rs.lineWidth);
	}

	if (!context.lastBoundPipeline || rs.pointSize != context.lastBoundPipeline->rasterizationState.pointSize)
	{
		glPointSize(rs.pointSize);
	}

	if (!context.lastBoundPipeline || rs.programPointSize != context.lastBoundPipeline->rasterizationState.programPointSize)
	{
		GLEnableOrDisable(GL_PROGRAM_POINT_SIZE, rs.programPointSize);
	}


	//////////////////////////////////////////////////////////////// depth + stencil
	const auto& ds = pipeline.depthState;
	if (!context.lastBoundPipeline || ds.depthTestEnable != context.lastBoundPipeline->depthState.depthTestEnable)
	{
		GLEnableOrDisable(GL_DEPTH_TEST, ds.depthTestEnable);
	}

	if (ds.depthTestEnable)
	{
		if (!context.lastBoundPipeline ||
			ds.depthWriteEnable != context.lastBoundPipeline->depthState.depthWriteEnable)
		{
			if (!context.lastBoundPipeline || ds.depthWriteEnable != context.lastBoundPipeline->depthState.depthWriteEnable)
			{
				glDepthMask(ds.depthWriteEnable);
			}
		}

		if (!context.lastBoundPipeline || ds.depthCompareOp != context.lastBoundPipeline->depthState.depthCompareOp)
		{
			glDepthFunc(CompareOpToGL(ds.depthCompareOp));
		}
	}

	const auto& ss = pipeline.stencilState;
	if (!context.lastBoundPipeline ||
		ss.stencilTestEnable != context.lastBoundPipeline->stencilState.stencilTestEnable)
	{
		GLEnableOrDisable(GL_STENCIL_TEST, ss.stencilTestEnable);
	}

	if (ss.stencilTestEnable)
	{
		//if (!context.lastBoundPipeline  || !context.lastBoundPipeline->stencilState.stencilTestEnable ||
		//    ss.front != context.lastBoundPipeline->stencilState.front)
		//{
		glStencilOpSeparate(GL_FRONT,
			StencilOpToGL(ss.front.failOp),
			StencilOpToGL(ss.front.depthFailOp),
			StencilOpToGL(ss.front.passOp));
		glStencilFuncSeparate(GL_FRONT, CompareOpToGL(ss.front.compareOp), ss.front.reference, ss.front.compareMask);
		//    if (context.lastStencilMask[0] != ss.front.writeMask)
		//    {
		glStencilMaskSeparate(GL_FRONT, ss.front.writeMask);
		//        context.lastStencilMask[0] = ss.front.writeMask;
		//    }
		//}

		//if (!context.lastBoundPipeline  || !context.lastBoundPipeline->stencilState.stencilTestEnable ||
		//    ss.back != context.lastBoundPipeline->stencilState.back)
		//{
		glStencilOpSeparate(GL_BACK,
			StencilOpToGL(ss.back.failOp),
			StencilOpToGL(ss.back.depthFailOp),
			StencilOpToGL(ss.back.passOp));
		glStencilFuncSeparate(GL_BACK, CompareOpToGL(ss.back.compareOp), ss.back.reference, ss.back.compareMask);
		//    if (context->lastStencilMask[1] != ss.back.writeMask)
		//    {
		glStencilMaskSeparate(GL_BACK, ss.back.writeMask);
		//        context->lastStencilMask[1] = ss.back.writeMask;
		//    }
		//}
	}

	//////////////////////////////////////////////////////////////// color blending state
	const auto& cb = pipeline.colorBlendState;
	if (!context.lastBoundPipeline || cb.logicOpEnable != context.lastBoundPipeline->colorBlendState.logicOpEnable)
	{
		GLEnableOrDisable(GL_COLOR_LOGIC_OP, cb.logicOpEnable);
		if (!context.lastBoundPipeline || !context.lastBoundPipeline->colorBlendState.logicOpEnable ||
			(cb.logicOpEnable && cb.logicOp != context.lastBoundPipeline->colorBlendState.logicOp))
		{
			glLogicOp(LogicOpToGL(cb.logicOp));
		}
	}

	if (!context.lastBoundPipeline || std::memcmp(cb.blendConstants,
		context.lastBoundPipeline->colorBlendState.blendConstants,
		sizeof(cb.blendConstants)) != 0)
	{
		glBlendColor(cb.blendConstants[0], cb.blendConstants[1], cb.blendConstants[2], cb.blendConstants[3]);
	}

	// FWOG_ASSERT((cb.attachments.empty()
	//   || (isRenderingToSwapchain && !cb.attachments.empty()))
	//   || lastRenderInfo->colorAttachments.size() >= cb.attachments.size()
	//   && "There must be at least a color blend attachment for each render target, or none");

	if (!context.lastBoundPipeline ||
		cb.attachments.empty() != context.lastBoundPipeline->colorBlendState.attachments.empty())
	{
		GLEnableOrDisable(GL_BLEND, !cb.attachments.empty());
	}

	for (GLuint i = 0; i < static_cast<GLuint>(cb.attachments.size()); i++)
	{
		const auto& cba = cb.attachments[i];
		if (context.lastBoundPipeline && i < context.lastBoundPipeline->colorBlendState.attachments.size() &&
			cba == context.lastBoundPipeline->colorBlendState.attachments[i])
		{
			continue;
		}

		if (cba.blendEnable)
		{
			glBlendFuncSeparatei(i,
				BlendFactorToGL(cba.srcColorBlendFactor),
				BlendFactorToGL(cba.dstColorBlendFactor),
				BlendFactorToGL(cba.srcAlphaBlendFactor),
				BlendFactorToGL(cba.dstAlphaBlendFactor));
			glBlendEquationSeparatei(i, BlendOpToGL(cba.colorBlendOp), BlendOpToGL(cba.alphaBlendOp));
		}
		else
		{
			// "no blending" blend state
			glBlendFuncSeparatei(i, GL_SRC_COLOR, GL_ZERO, GL_SRC_ALPHA, GL_ZERO);
			glBlendEquationSeparatei(i, GL_FUNC_ADD, GL_FUNC_ADD);
		}

		//if (context.lastColorMask[i] != cba.colorWriteMask)
		//{
		glColorMaski(i,
			(cba.colorWriteMask & ColorComponentFlag::R_BIT) != ColorComponentFlag::NONE,
			(cba.colorWriteMask & ColorComponentFlag::G_BIT) != ColorComponentFlag::NONE,
			(cba.colorWriteMask & ColorComponentFlag::B_BIT) != ColorComponentFlag::NONE,
			(cba.colorWriteMask & ColorComponentFlag::A_BIT) != ColorComponentFlag::NONE);
		//    context.lastColorMask[i] = cba.colorWriteMask;
		//}
	}

	context.lastBoundPipeline = pipeline;
	context.isPipelineBound = true;
}

void BindVBO(uint32 index, const Buffer<BufferType::VBO>& buffer, uint64 offset, uint64 stride)
{
	assert(context.isRendering && context.isPipelineBound);

	context.currentVao->bindVBO(index, buffer, offset, stride);
}

void BindEBO(const Buffer<BufferType::EBO>& buffer, IndexType indexType)
{
	assert(context.isRendering && context.isPipelineBound);
	context.isIndexBufferBound = true;
	context.currentIndexType = indexType;
	context.currentVao->bindEBO(buffer);
}

void BindSSBO(uint32 index, const Buffer<BufferType::SSBO>& buffer, uint64 offset, uint64 size)
{
	assert(context.isRendering && context.isPipelineBound);
	glBindBufferRange(GL_SHADER_STORAGE_BUFFER, index, buffer.getHandle(), offset, size);
}

void BindUBO(uint32 index, const Buffer<BufferType::UBO>& buffer, uint64 offset, uint64 size)
{
	assert(context.isRendering && context.isPipelineBound);
	glBindBufferRange(GL_UNIFORM_BUFFER, index, buffer.getHandle(), offset, size);
}

void BindSampledImage(uint32 index, const Texture& texture, const Sampler& sampler)
{
	assert(context.isRendering && context.isPipelineBound);
	glBindTextureUnit(index, texture.getHandle());
	glBindSampler(index, sampler.getHandle());
}

Shader& GetShader() {
	assert(context.isRendering && context.lastBoundPipeline && context.isPipelineBound);
	return *context.lastBoundPipeline->shader;
}

}
}