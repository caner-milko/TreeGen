#include "Tree.h"
#include "./TreeWorld.h"
#include "./Util.hpp"
#include <iostream>
#include <queue>
Tree::Tree(TreeWorld& world, vec3 position, uint32 seed) : world(world),
root(new TreeNode(nullptr, 0, position, vec3(0.0f, 1.0f, 0.0f))), seed(seed)
{
	budToMetamer(*root);
}

Tree::Tree(const Tree& from) : world(from.world), growthData(from.growthData), age(from.age), seed(from.seed), metamerCount(from.metamerCount), budCount(from.budCount)
{
	root = new TreeNode(*from.root);
	std::queue<TreeNode*> queue({ root });
	while (!queue.empty()) {
		TreeNode* selected = queue.front();
		queue.pop();
		if (selected->bud)
			continue;

		selected->mainChild = new TreeNode(*selected->mainChild);
		selected->mainChild->parent = selected;

		selected->lateralChild = new TreeNode(*selected->lateralChild);
		selected->lateralChild->parent = selected;

		queue.push(selected->lateralChild);
		queue.push(selected->mainChild);
	}
}

void Tree::budToMetamer(TreeNode& bud)
{
	bud.mainChild = new TreeNode(&bud, bud.id * 2 + 1, bud.endPos(), bud.direction);

	vec3 lateralDir = util::randomPerturbateVector(bud.direction, growthData.lateralAngle * 2.0f, world.seed + seed + age + bud.id * 2 + 2);
	bud.lateralChild = new TreeNode(&bud, bud.id * 2 + 2, bud.endPos(), lateralDir);

	bud.bud = false;
	bud.createdAt = age;
	metamerCount++;
	budCount++;
}

float Tree::accumulateLight()
{
	accumulateLightRecursive(*root);
	return root->light;
}

void Tree::distributeVigor()
{
	distributeVigorRecursive(*root);
}

float Tree::lightAtBud(TreeNode& bud)
{
	return world.getLightAt(bud.startPos, growthData.a, growthData.fullExposure);
}

void Tree::addNewShoots()
{
	addShootsRecursive(*root);
}

void Tree::printTreeRecursive(TreeNode& node, const std::string& prefix) const
{
	if (!node.bud) {
		vec3 start = node.startPos;
		vec3 end = node.endPos();
		std::cout << prefix << "Start: " << start.x << ", " << start.y << ", " << start.z << " End:" << end.x << ", " << end.y << ", " << end.z << " Light: " << node.light << std::endl;
		printTreeRecursive(*node.mainChild, prefix + " ");
		printTreeRecursive(*node.lateralChild, prefix + " ");
	}
}

void Tree::calculateShadows() const
{
	calculateShadowsRecursive(*root);
}

Tree::~Tree()
{
	std::queue<TreeNode*> queue({ root });
	while (!queue.empty()) {
		TreeNode* selected = queue.front();
		queue.pop();
		if (!selected->bud) {
			queue.push(selected->lateralChild);
			queue.push(selected->mainChild);
		}
		delete selected;
	}

}

float Tree::accumulateLightRecursive(TreeNode& node)
{
	if (node.bud)
	{
		node.light = lightAtBud(node);
	}
	else
	{
		node.light = accumulateLightRecursive(*node.mainChild) + accumulateLightRecursive(*node.lateralChild);
	}
	return node.light;
}

void Tree::distributeVigorRecursive(TreeNode& node)
{
	if (node.bud)
		return;
	float apicalControl = growthData.apicalControl;
	float mainV = apicalControl * node.mainChild->light;
	float lateralV = (1.0f - apicalControl) * node.lateralChild->light;
	float multiplier = node.vigor / (mainV + lateralV);
	node.mainChild->vigor = mainV * multiplier;
	node.lateralChild->vigor = lateralV * multiplier;
	distributeVigorRecursive(*node.mainChild);
	distributeVigorRecursive(*node.lateralChild);
}

void Tree::addShootsRecursive(TreeNode& node)
{
	if (!node.bud) {
		addShootsRecursive(*node.mainChild);
		addShootsRecursive(*node.lateralChild);
		return;
	}

	float vigor = node.vigor;
	int vigorFloored = static_cast<int>(glm::floor(vigor));

	float metamerLength = growthData.baseLength * vigor / glm::floor(vigor);

	vec3 optimal = world.getOptimalDirection(node.startPos, node.direction, growthData.perceptionRadius, growthData.perceptionAngle);

	vec3 direction = growthData.directionWeights.x * node.direction + growthData.directionWeights.y * optimal + growthData.directionWeights.z * growthData.tropism;

	TreeNode* current = &node;



	for (int i = 0; i < vigorFloored; i++) {
		current->length = metamerLength;
		current->direction = glm::normalize(direction);
		budToMetamer(*current);
		//TODO create shadows here?-
		direction = growthData.directionWeights.x * direction + growthData.directionWeights.y * optimal + growthData.directionWeights.z * growthData.tropism;
		current = current->mainChild;
	}
}

void Tree::calculateShadowsRecursive(TreeNode& node) const
{
	if (!node.bud) {
		calculateShadowsRecursive(*node.mainChild);
		calculateShadowsRecursive(*node.lateralChild);
	}
	else {
		world.castShadows(node.startPos, growthData.pyramidHeight, growthData.a, growthData.b);
	}
}
