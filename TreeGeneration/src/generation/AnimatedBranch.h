#pragma once
#include "TreeNode.h"
#include <glm/gtc/constants.hpp>
#include "util/BBox.h"
#include "Leaf.h"
#include <vector>
#include "Branch.h"
#include <optional>
namespace tgen::gen
{
//TODO move this to TreeRenderer
struct alignas(16) AnimatedBranchShaderData
{
	mat4 model;
	vec4 A;
	vec4 B;
	vec4 C;
	vec4 color;
	//lowRadius is animated from animationBounds.x to 1
	vec2 lowRadiusBounds;
	//if the branch is new, high radius stays 0 until animationT>animationBounds, then it is animated between animationBounds.y < animationT < 1
	vec2 highRadiusBounds;
	//bezier T bounds, both is 1 if the branch already exists, animated between animationBounds
	vec2 TBounds;
	//time where animation is active, 0-0 if branch already exists
	vec2 animationBounds;
	float startLength;
	float length;
	float offset;
	int order;
};

struct AnimatedBranch
{
	rb<const TreeNode> from;
	util::BBox boundingBox;
	AnimatedBranchShaderData shaderData;
public:
	AnimatedBranch(vec2 animationBounds, const Branch& endBranch,
		const std::optional<Branch>& startBranch = std::nullopt);
	AnimatedBranchShaderData asShaderData(const vec3& color) const;
};
}