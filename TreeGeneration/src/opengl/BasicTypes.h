#pragma once
#include "../Definitions.h"

//https://github.com/JuanDiegoMontoya/Fwog/blob/main/include/Fwog/BasicTypes.h

#define DECLARE_FLAG_TYPE(FLAG_TYPE, FLAG_BITS, BASE_TYPE)                           \
                                                                                          \
  struct FLAG_TYPE                                                                        \
  {                                                                                       \
    BASE_TYPE flags = static_cast<BASE_TYPE>(0);                                          \
                                                                                          \
    constexpr FLAG_TYPE() noexcept = default;                                             \
    constexpr explicit FLAG_TYPE(BASE_TYPE in) noexcept : flags(in) {}                    \
    constexpr FLAG_TYPE(FLAG_BITS in) noexcept : flags(static_cast<BASE_TYPE>(in)) {}     \
    constexpr bool operator==(FLAG_TYPE const& right) const                               \
    {                                                                                     \
      return flags == right.flags;                                                        \
    }                                                                                     \
    constexpr bool operator!=(FLAG_TYPE const& right) const                               \
    {                                                                                     \
      return flags != right.flags;                                                        \
    }                                                                                     \
    constexpr explicit operator BASE_TYPE() const                                         \
    {                                                                                     \
      return flags;                                                                       \
    }                                                                                     \
    constexpr explicit operator bool() const noexcept                                     \
    {                                                                                     \
      return flags != 0;                                                                  \
    }                                                                                     \
  };                                                                                      \
  constexpr FLAG_TYPE operator|(FLAG_TYPE a, FLAG_TYPE b) noexcept                        \
  {                                                                                       \
    return static_cast<FLAG_TYPE>(a.flags | b.flags);                                     \
  }                                                                                       \
  constexpr FLAG_TYPE operator&(FLAG_TYPE a, FLAG_TYPE b) noexcept                        \
  {                                                                                       \
    return static_cast<FLAG_TYPE>(a.flags & b.flags);                                     \
  }                                                                                       \
  constexpr FLAG_TYPE operator^(FLAG_TYPE a, FLAG_TYPE b) noexcept                        \
  {                                                                                       \
    return static_cast<FLAG_TYPE>(a.flags ^ b.flags);                                     \
  }                                                                                       \
  constexpr FLAG_TYPE operator~(FLAG_TYPE a) noexcept                                     \
  {                                                                                       \
    return static_cast<FLAG_TYPE>(~a.flags);                                              \
  }                                                                                       \
  constexpr FLAG_TYPE& operator|=(FLAG_TYPE& a, FLAG_TYPE b) noexcept                     \
  {                                                                                       \
    a.flags = (a.flags | b.flags);                                                        \
    return a;                                                                             \
  }                                                                                       \
  constexpr FLAG_TYPE& operator&=(FLAG_TYPE& a, FLAG_TYPE b) noexcept                     \
  {                                                                                       \
    a.flags = (a.flags & b.flags);                                                        \
    return a;                                                                             \
  }                                                                                       \
  constexpr FLAG_TYPE operator^=(FLAG_TYPE& a, FLAG_TYPE b) noexcept                      \
  {                                                                                       \
    a.flags = (a.flags ^ b.flags);                                                        \
    return a;                                                                             \
  }                                                                                       \
  constexpr FLAG_TYPE operator|(FLAG_BITS a, FLAG_BITS b) noexcept                        \
  {                                                                                       \
    return static_cast<FLAG_TYPE>(static_cast<BASE_TYPE>(a) | static_cast<BASE_TYPE>(b)); \
  }                                                                                       \
  constexpr FLAG_TYPE operator&(FLAG_BITS a, FLAG_BITS b) noexcept                        \
  {                                                                                       \
    return static_cast<FLAG_TYPE>(static_cast<BASE_TYPE>(a) & static_cast<BASE_TYPE>(b)); \
  }                                                                                       \
  constexpr FLAG_TYPE operator~(FLAG_BITS key) noexcept                                   \
  {                                                                                       \
    return static_cast<FLAG_TYPE>(~static_cast<BASE_TYPE>(key));                          \
  }                                                                                       \
  constexpr FLAG_TYPE operator^(FLAG_BITS a, FLAG_BITS b) noexcept                        \
  {                                                                                       \
    return static_cast<FLAG_TYPE>(static_cast<BASE_TYPE>(a) ^ static_cast<BASE_TYPE>(b)); \
  }

enum class ImageType : uint32_t
{
    TEX_1D,
    TEX_2D,
    TEX_3D,
    //TEX_1D_ARRAY,
    //TEX_2D_ARRAY,
    TEX_CUBEMAP,
    //TEX_CUBEMAP_ARRAY, // extremely cursed- do not use
    TEX_2D_MULTISAMPLE,
    //TEX_2D_MULTISAMPLE_ARRAY,
};

enum class Format : uint32_t
{
    UNDEFINED,
    R8_UNORM,
    R8_SNORM,
    R16_UNORM,
    R16_SNORM,
    R8G8_UNORM,
    R8G8_SNORM,
    R16G16_UNORM,
    R16G16_SNORM,
    R3G3B2_UNORM,
    R4G4B4_UNORM,
    R5G5B5_UNORM,
    R8G8B8_UNORM,
    R8G8B8_SNORM,
    R10G10B10_UNORM,
    R12G12B12_UNORM,
    R16G16B16_SNORM,
    R2G2B2A2_UNORM,
    R4G4B4A4_UNORM,
    R5G5B5A1_UNORM,
    R8G8B8A8_UNORM,
    R8G8B8A8_SNORM,
    R10G10B10A2_UNORM,
    R10G10B10A2_UINT,
    R12G12B12A12_UNORM,
    R16G16B16A16_UNORM,
    R16G16B16A16_SNORM,
    R8G8B8_SRGB,
    R8G8B8A8_SRGB,
    R16_FLOAT,
    R16G16_FLOAT,
    R16G16B16_FLOAT,
    R16G16B16A16_FLOAT,
    R32_FLOAT,
    R32G32_FLOAT,
    R32G32B32_FLOAT,
    R32G32B32A32_FLOAT,
    R11G11B10_FLOAT,
    R9G9B9_E5,
    R8_SINT,
    R8_UINT,
    R16_SINT,
    R16_UINT,
    R32_SINT,
    R32_UINT,
    R8G8_SINT,
    R8G8_UINT,
    R16G16_SINT,
    R16G16_UINT,
    R32G32_SINT,
    R32G32_UINT,
    R8G8B8_SINT,
    R8G8B8_UINT,
    R16G16B16_SINT,
    R16G16B16_UINT,
    R32G32B32_SINT,
    R32G32B32_UINT,
    R8G8B8A8_SINT,
    R8G8B8A8_UINT,
    R16G16B16A16_SINT,
    R16G16B16A16_UINT,
    R32G32B32A32_SINT,
    R32G32B32A32_UINT,

    D32_FLOAT,
    D32_UNORM,
    D24_UNORM,
    D16_UNORM,
    D32_FLOAT_S8_UINT,
    D24_UNORM_S8_UINT,
    // TODO: compressed formats
    // TODO: 64-bits-per-component formats
};

// multisampling and anisotropy
enum class SampleCount : uint8_t
{
    SAMPLES_1,
    SAMPLES_2,
    SAMPLES_4,
    SAMPLES_8,
    SAMPLES_16,
};

enum class UploadDimension : uint32_t
{
    ONE,
    TWO,
    THREE,
};

enum class UploadFormat : uint32_t
{
    UNDEFINED,
    R,
    RG,
    RGB,
    BGR,
    RGBA,
    BGRA,
    R_INTEGER,
    RG_INTEGER,
    RGB_INTEGER,
    BGR_INTEGER,
    RGBA_INTEGER,
    BGRA_INTEGER,
    DEPTH_COMPONENT,
    STENCIL_INDEX,
    DEPTH_STENCIL,
};

enum class UploadType : uint32_t
{
    UNDEFINED,
    UBYTE,
    SBYTE,
    USHORT,
    SSHORT,
    UINT,
    SINT,
    FLOAT,
    UBYTE_3_3_2,
    UBYTE_2_3_3_REV,
    USHORT_5_6_5,
    USHORT_5_6_5_REV,
    USHORT_4_4_4_4,
    USHORT_4_4_4_4_REV,
    USHORT_5_5_5_1,
    USHORT_1_5_5_5_REV,
    UINT_8_8_8_8,
    UINT_8_8_8_8_REV,
    UINT_10_10_10_2,
    UINT_2_10_10_10_REV,
};

enum class Filter : uint8_t
{
    NONE,
    NEAREST,
    LINEAR,
    NEAREST_MIPMAP_NEAREST,
    LINEAR_MIPMAP_NEAREST,
    NEAREST_MIPMAP_LINEAR,
    LINEAR_MIPMAP_LINEAR,
};

enum class AddressMode : uint8_t
{
    REPEAT,
    MIRRORED_REPEAT,
    CLAMP_TO_EDGE,
    CLAMP_TO_BORDER,
    MIRROR_CLAMP_TO_EDGE,
};

enum class BorderColor : uint8_t
{
    FLOAT_TRANSPARENT_BLACK,
    INT_TRANSPARENT_BLACK,
    FLOAT_OPAQUE_BLACK,
    INT_OPAQUE_BLACK,
    FLOAT_OPAQUE_WHITE,
    INT_OPAQUE_WHITE,
};

enum class AspectMaskBit : uint32_t
{
    COLOR_BUFFER_BIT = 1 << 0,
    DEPTH_BUFFER_BIT = 1 << 1,
    STENCIL_BUFFER_BIT = 1 << 2,
};
DECLARE_FLAG_TYPE(AspectMask, AspectMaskBit, uint32_t)

enum class PrimitiveTopology : uint32_t
{
    POINT_LIST,
    LINE_LIST,
    LINE_STRIP,
    TRIANGLE_LIST,
    TRIANGLE_STRIP,
    TRIANGLE_FAN,
    // TODO: add more toplogies that are deemed useful
};

enum class PolygonMode : uint32_t
{
    FILL,
    LINE,
    POINT,
};

enum class CullMode : uint32_t
{
    NONE = 0b00,
    FRONT = 0b01,
    BACK = 0b10,
    FRONT_AND_BACK = 0b11,
};

enum class FrontFace : uint32_t
{
    CLOCKWISE,
    COUNTERCLOCKWISE,
};

enum class CompareOp : uint8_t
{
    NEVER,
    LESS,
    EQUAL,
    LESS_OR_EQUAL,
    GREATER,
    NOT_EQUAL,
    GREATER_OR_EQUAL,
    ALWAYS,
};

enum class LogicOp : uint32_t
{
    CLEAR,
    SET,
    COPY,
    COPY_INVERTED,
    NO_OP,
    INVERT,
    AND,
    NAND,
    OR,
    NOR,
    XOR,
    EQUIVALENT,
    AND_REVERSE,
    OR_REVERSE,
    AND_INVERTED,
    OR_INVERTED,
};

enum class BlendFactor : uint32_t
{
    ZERO,
    ONE,
    SRC_COLOR,
    ONE_MINUS_SRC_COLOR,
    DST_COLOR,
    ONE_MINUS_DST_COLOR,
    SRC_ALPHA,
    ONE_MINUS_SRC_ALPHA,
    DST_ALPHA,
    ONE_MINUS_DST_ALPHA,
    CONSTANT_COLOR,
    ONE_MINUS_CONSTANT_COLOR,
    CONSTANT_ALPHA,
    ONE_MINUS_CONSTANT_ALPHA,
    SRC_ALPHA_SATURATE,
    SRC1_COLOR,
    ONE_MINUS_SRC1_COLOR,
    SRC1_ALPHA,
    ONE_MINUS_SRC1_ALPHA,
};

enum class BlendOp : uint32_t
{
    ADD,
    SUBTRACT,
    REVERSE_SUBTRACT,
    MIN,
    MAX,
};

enum class ColorComponentFlag : uint32_t
{
    NONE,
    R_BIT = 0b0001,
    G_BIT = 0b0010,
    B_BIT = 0b0100,
    A_BIT = 0b1000,
    RGBA_BITS = 0b1111,
};
DECLARE_FLAG_TYPE(ColorComponentFlags, ColorComponentFlag, uint32_t)

enum class IndexType : uint32_t
{
    UNSIGNED_BYTE,
    UNSIGNED_SHORT,
    UNSIGNED_INT,
};

enum class MemoryBarrierBit : uint32_t
{
    NONE = 0,
    VERTEX_BUFFER_BIT = 1 << 0,  // GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT
    INDEX_BUFFER_BIT = 1 << 1,  // GL_ELEMENT_ARRAY_BARRIER_BIT
    UNIFORM_BUFFER_BIT = 1 << 2,  // GL_UNIFORM_BARRIER_BIT
    TEXTURE_FETCH_BIT = 1 << 3,  // GL_TEXTURE_FETCH_BARRIER_BIT
    IMAGE_ACCESS_BIT = 1 << 4,  // GL_SHADER_IMAGE_ACCESS_BARRIER_BIT
    COMMAND_BUFFER_BIT = 1 << 5,  // GL_COMMAND_BARRIER_BIT
    TEXTURE_UPDATE_BIT = 1 << 6,  // GL_TEXTURE_UPDATE_BARRIER_BIT
    BUFFER_UPDATE_BIT = 1 << 7,  // GL_BUFFER_UPDATE_BARRIER_BIT
    MAPPED_BUFFER_BIT = 1 << 8,  // GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT
    FRAMEBUFFER_BIT = 1 << 9,  // GL_FRAMEBUFFER_BARRIER_BIT
    SHADER_STORAGE_BIT = 1 << 10, // GL_SHADER_STORAGE_BARRIER_BIT
    QUERY_COUNTER_BIT = 1 << 11, // GL_QUERY_BUFFER_BARRIER_BIT
    ALL_BITS = static_cast<uint32_t>(-1),
    // TODO: add more bits as necessary
};
DECLARE_FLAG_TYPE(MemoryBarrierBits, MemoryBarrierBit, uint32_t)

enum class StencilOp : uint32_t
{
    KEEP = 0,
    ZERO = 1,
    REPLACE = 2,
    INCREMENT_AND_CLAMP = 3,
    DECREMENT_AND_CLAMP = 4,
    INVERT = 5,
    INCREMENT_AND_WRAP = 6,
    DECREMENT_AND_WRAP = 7,
};

struct DrawIndirectCommand
{
    uint32_t vertexCount;
    uint32_t instanceCount;
    uint32_t firstVertex;
    uint32_t firstInstance;
};

struct DrawIndexedIndirectCommand
{
    uint32_t indexCount;
    uint32_t instanceCount;
    uint32_t firstIndex;
    int32_t vertexOffset;
    uint32_t firstInstance;
};

struct DispatchIndirectCommand
{
    uint32_t groupCountX;
    uint32_t groupCountY;
    uint32_t groupCountZ;
};

enum class ClipDepthRange
{
    NegativeOneToOne, // OpenGL default
    ZeroToOne         // D3D and Vulkan
};

enum class BufferStorageFlag : uint32_t
{
    NONE = 0,
    DYNAMIC_STORAGE = 1 << 0,
    CLIENT_STORAGE = 1 << 1,
};
DECLARE_FLAG_TYPE(BufferStorageFlags, BufferStorageFlag, uint32_t)

enum class BufferMapFlag : uint32_t
{
    NONE = 0,
    MAP_READ = 1 << 0,
    MAP_WRITE = 1 << 1,
    MAP_PERSISTENT = 1 << 2,
    MAP_COHERENT = 1 << 3,
};
DECLARE_FLAG_TYPE(BufferMapFlags, BufferMapFlag, uint32_t)
