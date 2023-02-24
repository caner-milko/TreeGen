#pragma once
#include "Pipeline.h"
#include "types/Buffer.h"
#include "Rendering.h"
namespace tgen::graphics {
struct Vertex {
	vec3 pos;
	vec2 texCoords;
	vec3 normal;
	static constexpr gl::VertexInputBindingDescription bindingDescription[] = {
		{.location = 0, .binding = 0,
		.format = gl::Format::R32G32B32_FLOAT, .offset = 0},
		{.location = 1, .binding = 0,
		.format = gl::Format::R32G32_FLOAT, .offset = sizeof(vec3)},
		{.location = 2, .binding = 0,
		.format = gl::Format::R32G32B32_FLOAT, .offset = sizeof(vec3) + sizeof(vec2)},
	};
};
template<typename T>
struct ArrayMesh {
	gl::VBO<T> vbo;
	gl::VertexInputState inputState;
};
template<enum gl::IndexType T>
struct IndexedMesh;

template<>
struct IndexedMesh<gl::IndexType::UNSIGNED_BYTE> {
	gl::EBOByte ebo;
};

template<>
struct IndexedMesh<gl::IndexType::UNSIGNED_SHORT> {
	gl::EBOShort ebo;
};

template<>
struct IndexedMesh<gl::IndexType::UNSIGNED_INT> {
	gl::EBOInt ebo;
};

template<typename T, enum gl::IndexType I>
struct CompleteMesh : public ArrayMesh<T>, IndexedMesh<I> {
};
namespace gl::Cmd::util {
template<typename T>
static void BindMesh(const ArrayMesh<T>& mesh) {
	Cmd::BindVBO(0, mesh.vbo, 0, sizeof(T));
}
template<enum gl::IndexType I>
static void BindMesh(const IndexedMesh<I>& mesh) {
	Cmd::BindEBO(mesh.ebo, I);
}
template<typename T, enum gl::IndexType I>
static void BindMesh(const CompleteMesh<T, I>& mesh) {
	BindMesh(static_cast<const ArrayMesh<T>&>(mesh));
	BindMesh(static_cast<const IndexedMesh<I>&>(mesh));
}
}
}