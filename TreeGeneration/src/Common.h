#pragma once
#include <cstdint>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp> 
#include <glm/gtx/quaternion.hpp>
#include <memory>
namespace tgen {
typedef uint8_t uint8;
typedef int8_t int8;
typedef uint16_t uint16;
typedef int16_t int16;
typedef uint32_t uint32;
typedef int32_t int32;
typedef uint64_t uint64;
typedef int64_t int64;
typedef glm::vec4 vec4;
typedef glm::vec3 vec3;
typedef glm::vec2 vec2;
typedef glm::ivec4 ivec4;
typedef glm::ivec3 ivec3;
typedef glm::ivec2 ivec2;
typedef glm::uvec4 uvec4;
typedef glm::uvec3 uvec3;
typedef glm::uvec2 uvec2;
typedef glm::mat4 mat4;
typedef glm::mat3 mat3;
typedef vec3 Color;
typedef glm::quat quat;
const float PI = glm::pi<float>();

template<typename T>
using rc = std::shared_ptr<T>;
template<typename T>
using ru = std::unique_ptr<T>;
template<typename T>
using rw = std::weak_ptr<T>;
//non-owning ptr
template<typename T>
using rb = T*;


#define DELETE_COPY_CONSTRUCTORS(TYPE) \
TYPE(const TYPE&) = delete;            \
TYPE& operator=(const TYPE&) = delete; \
TYPE& operator=(TYPE&) = delete;       \


#define MOVE_OPERATOR(TYPE) \
TYPE& operator=(TYPE&& other) noexcept  \
{if(&other == this) return *this; this->~TYPE(); return *new (this) TYPE(std::move(other));}

#define GL_OPERATOR_BOOL(HANDLE_NAME) \
explicit operator bool() const {return HANDLE_NAME != 0;}
}