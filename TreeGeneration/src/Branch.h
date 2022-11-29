#pragma once
#include "./TreeNode.h"
#include <glm/gtc/constants.hpp>



struct Branch {
	vec3 A, B, C;
	float lowRadius;
	float highRadius;
	vec3 bezierPlaneNormal;
	float startLength;
	float length;
	float offset;
	const TreeNode& from;


public:
	Branch(const TreeNode& node, float startLength, const vec3& lastPlaneNormal, float lastOffset);
};