#pragma once
#include "./Definitions.h"
#include <memory>

struct TreeNode;

struct TreeNode
{
	vec3 startPos;
	vec3 direction;

	float length = 1.0f;
	float radius = 0.0f;

	bool bud = true;
	uint32 id = 0;
	uint32 order = 0;
	uint32 createdAt = 0;

	//iteration parameters
	float light = 0.0f;
	float vigor = 0.0f;


	//root if null
	TreeNode* parent;

	TreeNode* mainChild = nullptr;

	TreeNode* lateralChild = nullptr;

	TreeNode(TreeNode* parent, uint32 id, vec3 startPos, vec3 direction) : startPos(startPos), id(id), direction(direction), parent(parent)
	{
		if (parent != nullptr)
			order = parent->order + 1;
	}

	vec3 endPos() const
	{
		return startPos + length * direction;
	}
};
