#include "Tree.h"
#include "./TreeWorld.h"
#include "./Util.hpp"
#include <iostream>
#include <queue>
#include <stack>
Tree::Tree(TreeWorld& world, uint32 id, vec3 position, TreeGrowthData growthData, uint32 seed) : world(world), id(id), growthData(growthData),
root(new TreeNode(nullptr, 0, position, vec3(0.0f, 1.0f, 0.0f))), seed(seed)
{

}

Tree::Tree(const Tree& from) : world(from.world), id(from.id), growthData(from.growthData), age(from.age), seed(from.seed), metamerCount(from.metamerCount), budCount(from.budCount)
{
	root = new TreeNode(*from.root);
	std::queue<TreeNode*> queue({ root });
	while (!queue.empty()) {
		TreeNode* selected = queue.front();
		queue.pop();
		if (selected->nodeStatus != ALIVE)
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
	bud.mainChild = new TreeNode(&bud, lastNodeId, bud.endPos(), bud.direction);
	lastNodeId++;
	vec3 lateralDir = util::randomPerturbateVector(glm::normalize(bud.direction), growthData.lateralAngle, world.seed + seed + age + bud.id * 2 + 2);
	bud.lateralChild = new TreeNode(&bud, lastNodeId, bud.endPos(), lateralDir);
	budCount++;
	lastNodeId++;

	bud.nodeStatus = ALIVE;
	bud.createdAt = age;
	metamerCount++;
	maxOrder = glm::max(maxOrder, bud.order + 1);
}

void Tree::startGrow()
{
	branchs.clear();
}

float Tree::accumulateLight()
{
	accumulateLightRecursive(*root);
	return root->light;
}

void Tree::distributeVigor()
{
	root->vigor = growthData.vigorMultiplier * root->light;
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

void Tree::shedBranchs()
{
	shedBranchsRecursive(*root);
}

void Tree::calculateChildCount()
{
	calculateChildCountRecursive(*root);
}

void Tree::endGrow()
{
}

void Tree::printTreeRecursive(TreeNode& node, const std::string& prefix) const
{
	if (node.nodeStatus == ALIVE) {
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

std::vector<TreeNode> Tree::AsNodeVector(bool includeBuds) const
{
	if (root->nodeStatus != ALIVE)
		return {};
	std::vector<TreeNode> nodes;
	std::queue<const TreeNode*> queue({ root });
	while (!queue.empty()) {
		const TreeNode* selected = queue.front();
		queue.pop();
		if (selected->nodeStatus == ALIVE) {
			if (includeBuds || selected->mainChild->nodeStatus == ALIVE) {
				queue.push(selected->mainChild);
			}
			if (includeBuds || selected->lateralChild->nodeStatus == ALIVE) {
				queue.push(selected->lateralChild);
			}
		}
		nodes.push_back(*selected);
	}

	return nodes;
}

const std::vector<Branch>& Tree::getBranchs()
{
	if (!branchs.empty())
		return branchs;
	return recalculateBranchs();
}

const std::vector<Branch>& Tree::recalculateBranchs() {
	branchs.clear();
	if (root->nodeStatus != ALIVE)
		return branchs;
	std::stack<const TreeNode*> stack({ root });
	float lastOffset = 0.0f;
	vec3 lastPlaneNormal(1.0f, 0.0f, 0.0f);
	float length = 0.0f;
	while (!stack.empty()) {
		const TreeNode* selected = stack.top();
		stack.pop();

		bool branchEnd = true;

		const TreeNode* weakChild = selected->weakerChild();
		if (weakChild != nullptr) {
			stack.push(weakChild);
		}
		const TreeNode* domChild = selected->dominantChild();
		if (domChild != nullptr) {
			stack.push(domChild);
			branchEnd = false;
		}

		Branch branch(*selected, growthData.baseRadius, growthData.radiusN, growthData.branchCurviness, length, lastPlaneNormal, lastOffset);

		if (branchEnd) {
			lastOffset = 0.0f;
			lastPlaneNormal = vec3(1.0f, 0.0f, 0.0f);
			length = 0.0f;
		}
		else {
			lastOffset = branch.offset;
			lastPlaneNormal = branch.bezierPlaneNormal;
			length += branch.length;
		}
		branchs.push_back(branch);
	}
	generateLeaves();
	return branchs;
}

void Tree::generateLeaves()
{
	for (auto& branch : branchs) {
		branch.generateLeaves(growthData.leafMaxChildCount, growthData.leafMinOrder, growthData.leafDensity, growthData.leafSizeMultiplier);
	}
}

Tree::~Tree()
{
	std::queue<TreeNode*> queue({ root });
	while (!queue.empty()) {
		TreeNode* selected = queue.front();
		queue.pop();
		if (selected->nodeStatus == ALIVE) {
			queue.push(selected->lateralChild);
			queue.push(selected->mainChild);
		}
		delete selected;
	}

}

bool Tree::operator==(const Tree& other) const
{
	return other.id == id && other.age == age;
}

float Tree::accumulateLightRecursive(TreeNode& node)
{
	if (node.nodeStatus == BUD)
	{
		if (!world.isOutOfBounds(node.startPos))
			node.light = lightAtBud(node);
	}
	else if (node.nodeStatus == ALIVE)
	{
		node.light = accumulateLightRecursive(*node.mainChild) + accumulateLightRecursive(*node.lateralChild);
	}
	else {
		node.light = 0.0f;
	}
	return node.light;
}

void Tree::distributeVigorRecursive(TreeNode& node)
{
	if (node.nodeStatus != ALIVE)
		return;
	float apicalControl = growthData.apicalControl;
	float mainV = 0.0f;
	float lateralV = 0.0f;
	if (node.mainChild->nodeStatus != DEAD) {
		mainV = apicalControl * node.mainChild->light;
	}
	if (node.lateralChild->nodeStatus != DEAD) {
		lateralV = (1.0f - apicalControl) * node.lateralChild->light;
	}
	float tot = mainV + lateralV;
	if (tot == 0.0f)
		return;
	float multiplier = node.vigor / (tot);
	node.mainChild->vigor = mainV * multiplier;
	node.lateralChild->vigor = lateralV * multiplier;
	distributeVigorRecursive(*node.mainChild);
	distributeVigorRecursive(*node.lateralChild);
}

void Tree::addShootsRecursive(TreeNode& node)
{
	if (node.nodeStatus == ALIVE) {
		addShootsRecursive(*node.mainChild);
		addShootsRecursive(*node.lateralChild);
		return;
	}
	else if (node.nodeStatus == DEAD) {
		return;
	}

	float vigor = node.vigor;
	int vigorFloored = static_cast<int>(glm::floor(vigor));

	float metamerLength = growthData.baseLength * vigor / glm::floor(vigor);

	if (world.isOutOfBounds(node.startPos))
		return;

	vec3 optimal = world.getOptimalDirection(node.startPos);

	vec3 direction = node.direction;

	TreeNode* current = &node;


	for (int i = 0; i < vigorFloored; i++) {
		if (world.isOutOfBounds(current->startPos))
			break;

		direction = glm::normalize(direction + growthData.directionWeights.x * optimal + growthData.directionWeights.y * growthData.tropism);
		current->length = metamerLength;
		current->direction = direction;
		budToMetamer(*current);
		//TODO create shadows here?-
		//optimal = world.getOptimalDirection(current->startPos, current->direction, growthData.perceptionRadius, growthData.perceptionAngle);
		current = current->mainChild;
	}
}

void Tree::shedBranchsRecursive(TreeNode& node)
{
	if (node.nodeStatus != ALIVE)
		return;

	float p = node.vigor - growthData.shedMultiplier *
		glm::pow(glm::pow(node.childCount, 1.0f / growthData.radiusN), growthData.shedExp);

	if (age - node.createdAt < 4 || p >= 0)
	{
		shedBranchsRecursive(*node.mainChild);
		shedBranchsRecursive(*node.lateralChild);
		return;
	}

	node.nodeStatus = DEAD;
	std::queue<TreeNode*> query({ node.mainChild, node.lateralChild });
	while (!query.empty()) {
		TreeNode* sel = query.front();
		if (node.nodeStatus == ALIVE) {
			query.emplace(sel->mainChild);
			query.emplace(sel->lateralChild);
		}
		query.pop();
		delete sel;
	}
}

float Tree::calculateChildCountRecursive(TreeNode& node)
{
	if (node.nodeStatus == BUD)
		return 1;
	else if (node.nodeStatus == DEAD) {
		return 0;
	}
	else {
		uint32 newChildCount = calculateChildCountRecursive(*node.mainChild) + calculateChildCountRecursive(*node.lateralChild);
		node.childCount = node.childCount > newChildCount ? node.childCount : newChildCount;
		return node.childCount;
	}
}

void Tree::calculateShadowsRecursive(TreeNode& node) const
{
	if (node.nodeStatus == ALIVE) {
		calculateShadowsRecursive(*node.mainChild);
		calculateShadowsRecursive(*node.lateralChild);
	}
	else if (node.nodeStatus == BUD) {
		if (!world.isOutOfBounds(node.startPos)) {
			if (node.order == 0 || node.parent->mainChild->id == node.id)
				world.castShadows(node.startPos, growthData.pyramidHeight, growthData.a, growthData.b);
		}
	}
}
