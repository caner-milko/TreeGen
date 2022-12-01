#pragma once
#include "./TreeNode.h"
#include <glm/gtc/constants.hpp>
#include "BBox.h"
#include "Leaf.h"
#include <vector>
struct Branch {
	vec3 A, B, C;
	float lowRadius;
	float highRadius;
	vec3 bezierPlaneNormal;
	float startLength;
	float length;
	float offset;
	const TreeNode& from;
	BBox boundingBox;
	std::vector<Leaf> leaves;

public:
	Branch(const TreeNode& node, float startLength, const vec3& lastPlaneNormal, float lastOffset);

	void generateLeaves(float maxWidth, float leafDensity);

	vec3 evaluatePos(float t) inline const;
	vec3 evaluateDir(float t) inline const;
	vec3 evaluateNormal(float t) inline const;
	float evaluateWidth(float t) inline const;
};