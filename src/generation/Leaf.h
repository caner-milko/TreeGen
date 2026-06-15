#pragma once
#include "Common.h"
namespace tgen::gen
{
struct Branch;
struct AnimatedBranch;

struct Leaf
{
	mat4 model;
	rb<Branch> attachedTo;
	float branchT;
	float randomAngle;
	float size;
	static float pertubateAngle;

	Leaf(rb<Branch> branch, float branchT, float size, float randomAngle);

	mat4 animatedLeaf(rb<AnimatedBranch> animated, uint32 treeAge, float animationT);

	vec3 calculatePos() inline const;
};
}