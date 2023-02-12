#pragma once
#include "types/Shader.h"
#include "BasicTypes.h"
#include <span>
namespace tgen::graphics::gl {

struct InputAssemblyState
{
    PrimitiveTopology topology = PrimitiveTopology::TRIANGLE_LIST;
    bool primitiveRestartEnable = false;
};

struct VertexInputBindingDescription
{
    uint32_t location; // glEnableVertexArrayAttrib + glVertexArrayAttribFormat
    uint32_t binding;  // glVertexArrayAttribBinding
    Format format;     // glVertexArrayAttribFormat
    uint32_t offset;   // glVertexArrayAttribFormat
};

struct VertexInputState
{
    std::span<const VertexInputBindingDescription> vertexBindingDescriptions = {};
};

struct RasterizationState
{
    bool depthClampEnable = false;
    PolygonMode polygonMode = PolygonMode::FILL;
    CullMode cullMode = CullMode::BACK;
    FrontFace frontFace = FrontFace::COUNTERCLOCKWISE;
    bool depthBiasEnable = false;
    float depthBiasConstantFactor = 0;
    float depthBiasSlopeFactor = 0;
    float lineWidth = 1; // glLineWidth
    float pointSize = 1; // glPointSize
    bool programPointSize = false;
};

struct DepthState
{
    bool depthTestEnable = true;            // gl{Enable, Disable}(GL_DEPTH_TEST)
    bool depthWriteEnable = true;            // glDepthMask(depthWriteEnable)
    CompareOp depthCompareOp = CompareOp::LESS; // glDepthFunc
};

struct StencilOpState
{
    StencilOp passOp = StencilOp::KEEP;   // glStencilOp (dppass)
    StencilOp failOp = StencilOp::KEEP;   // glStencilOp (sfail)
    StencilOp depthFailOp = StencilOp::KEEP;   // glStencilOp (dpfail)
    CompareOp compareOp = CompareOp::ALWAYS; // glStencilFunc (func)
    uint32 compareMask = 0;                 // glStencilFunc (mask)
    uint32 writeMask = 0;                 // glStencilMask
    uint32 reference = 0;                 // glStencilFunc (ref)
};

struct StencilState
{
    bool stencilTestEnable = false;
    StencilOpState front = {};
    StencilOpState back = {};
};

struct ColorBlendAttachmentState      // glBlendFuncSeparatei + glBlendEquationSeparatei
{
    bool blendEnable = false;                                           // if false, blend factor = one?
    BlendFactor srcColorBlendFactor = BlendFactor::ONE;              // srcRGB
    BlendFactor dstColorBlendFactor = BlendFactor::ZERO;             // dstRGB
    BlendOp colorBlendOp = BlendOp::ADD;                  // modeRGB
    BlendFactor srcAlphaBlendFactor = BlendFactor::ONE;              // srcAlpha
    BlendFactor dstAlphaBlendFactor = BlendFactor::ZERO;             // dstAlpha
    BlendOp alphaBlendOp = BlendOp::ADD;                  // modeAlpha
    ColorComponentFlags colorWriteMask = ColorComponentFlag::RGBA_BITS; // glColorMaski

    bool operator==(const ColorBlendAttachmentState&) const = default;
};

struct ColorBlendState
{
    bool logicOpEnable = false;          // gl{Enable, Disable}(GL_COLOR_LOGIC_OP)
    LogicOp logicOp = LogicOp::COPY;  // glLogicOp(logicOp)
    std::span<const ColorBlendAttachmentState> attachments = {};             // glBlendFuncSeparatei + glBlendEquationSeparatei
    float blendConstants[4] = { 0, 0, 0, 0 }; // glBlendColor
};

struct GraphicsPipeline {
    GraphicsPipeline(std::string_view name, Shader& shader) : name(name), shader(&shader) {
        assert(shader);
    }
    std::string name;
    rb<Shader> shader;
    InputAssemblyState inputAssemblyState = {};
    VertexInputState vertexInputState = {};
    RasterizationState rasterizationState = {};
    DepthState depthState = {};
    StencilState stencilState = {};
    ColorBlendState colorBlendState = {};
};
}