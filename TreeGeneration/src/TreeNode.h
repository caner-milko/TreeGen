#pragma once
#include "./Definitions.h"
#include <memory>

struct TreeNode;

typedef std::unique_ptr<TreeNode> UPtrTreeNode;

struct TreeNode
{
	vec3 startPos;
	vec3 direction;

	float length = 1.0f;
	float diameter = 0.1f;

	bool bud = true;
	uint32 order = 0;

	//iteration parameters
	float light = 0.0f;
	float vigor = 0.0f;


	//root if null
	TreeNode* parent;

	UPtrTreeNode mainChild;

	UPtrTreeNode lateralChild;


	TreeNode(TreeNode* parent, vec3 startPos, vec3 direction) : startPos(startPos), direction(direction), parent(parent)
	{
		if (parent != nullptr)
			order = parent->order + 1;
	}

	vec3 endPos() const
	{
		return startPos + length * direction;
	}
};
