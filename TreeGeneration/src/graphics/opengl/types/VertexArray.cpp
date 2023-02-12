#include "VertexArray.h"
#include "ApiToEnum.h"
namespace tgen::graphics::gl {
    using namespace tgen::graphics::gl::detail;
void VertexArray::enableAttribute(uint32 location, uint64 binding, Format format, uint64 offset) {
	assert(handle);
	glEnableVertexArrayAttrib(handle, location);
	glVertexArrayAttribBinding(handle, location, binding);

    auto type = FormatToTypeGL(format);
    auto size = FormatToSizeGL(format);
    auto normalized = IsFormatNormalizedGL(format);
    auto internalType = FormatToFormatClass(format);
    switch (internalType)
    {
    case GlFormatClass::FLOAT: 
        glVertexArrayAttribFormat(handle, location, size, type, normalized, offset); break;
    case GlFormatClass::INT:
        glVertexArrayAttribIFormat(handle, location, size, type, offset); break;
    case GlFormatClass::LONG:
        glVertexArrayAttribLFormat(handle, location, size, type, offset); break;
    default: assert(false);
    }

}

void VertexArray::bindVBO(uint32 index, const Buffer<BufferType::VBO>& buffer, uint64 offset, uint64 stride) {
    assert(handle && buffer.getHandle() && offset <= buffer.getRawSize());
    glVertexArrayVertexBuffer(handle,
        index,
        buffer.getHandle(),
        offset,
        stride);
}

void VertexArray::bindEBO(const Buffer<BufferType::EBO>& buffer) {
    assert(handle && buffer.getHandle());
    glVertexArrayElementBuffer(handle, buffer.getHandle());

}

}