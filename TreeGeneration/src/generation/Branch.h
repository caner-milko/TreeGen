#pragma once
#include "TreeNode.h"
#include <glm/gtc/constants.hpp>
#include "util/BBox.h"
#include "Leaf.h"
#include <vector>
namespace tgen::gen
{
//TODO move this to TreeRenderer
struct BranchShaderData
{
	mat4 model;
	vec4 A;
	vec4 B;
	vec4 C;
	vec4 color;

	float lowRadius;
	float highRadius;
	float startLength;
	float length;

	float offset;
	int order;

	vec2 a;

	//vec4 filler;
};

struct Branch
{
	mat4 model;
	vec3 A, B, C;
	float lowRadius;
	float highRadius;
	vec3 bezierPlaneNormal;
	float startLength;
	float length;
	float offset;
	int order;

	util::BBox boundingBox;
	std::vector<Leaf> leaves;
	rb<const TreeNode> from;

public:
	Branch(rb<const TreeNode> node, float baseRadius, float radiusPow, float curviness, float startLength, const vec3& lastPlaneNormal, float lastOffset);

	Branch(const Branch& other, bool copyLeaves)
		: model(other.model), A(other.A), B(other.B), C(other.C), lowRadius(other.lowRadius), highRadius(other.highRadius),
		bezierPlaneNormal(other.bezierPlaneNormal), startLength(other.startLength), length(other.length),
		offset(other.offset), order(other.order), boundingBox(other.boundingBox), from(other.from)
	{
		if (copyLeaves)
			leaves = other.leaves;
	}

	void generateLeaves(uint32 maxChildCount, uint32 minOrder, float leafDensity, float sizeMultiplier);

	BranchShaderData asShaderData(const vec3& color) const;

	vec3 evaluatePos(float t) inline const;
	vec3 evaluateDir(float t) inline const;
	vec3 evaluateNormal(float t) inline const;
	float evaluateWidth(float t) inline const;
};
}