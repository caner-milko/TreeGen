#pragma once
#include "Common.h"
namespace tgen::gen{
struct TreeNode
{
	enum NodeStatus {
		BUD, ALIVE, DEAD,
	};
	vec3 startPos;
	vec3 direction;
	float length = 1.0f;

	uint32 childCount = 0;

	uint8 nodeStatus = NodeStatus::BUD;
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

	TreeNode* dominantChild() const {
		if (mainChild->nodeStatus != ALIVE) {
			if (lateralChild->nodeStatus != ALIVE)
				return nullptr;
			return lateralChild;
		}
		if (lateralChild->nodeStatus != ALIVE)
			return mainChild;
		if (mainChild->childCount >= lateralChild->childCount)
			return mainChild;
		return lateralChild;
	}

	TreeNode* weakerChild() const {
		if (mainChild->nodeStatus != ALIVE || lateralChild->nodeStatus != ALIVE) {
			return nullptr;
		}
		if (mainChild->childCount >= lateralChild->childCount)
			return lateralChild;
		return mainChild;
	}

	bool isDominantChild() const {
		return order == 0 || parent->dominantChild()->id == id;
	}

	TreeNode* sibling() const {
		if (order == 0)
			return nullptr;
		if (parent->lateralChild == this)
			return parent->mainChild;
		return parent->lateralChild;
	}
};
}