#include "Tree.h"
#include "TreeWorld.h"
#include "util/Util.h"
#include <iostream>
#include <queue>
#include <stack>
namespace tgen::gen
{
Tree::Tree(rb<TreeWorld> world, uint32 id, vec3 position, TreeGrowthData growthData, uint32 seed) : world(world), id(id), growthData(growthData),
root(new TreeNode(nullptr, 0, position, vec3(0.0f, 1.0f, 0.0f))), seed(seed)
{
	addShadows(*root);
}

Tree::Tree(const Tree& from)
	: world(from.world), id(from.id), growthData(from.growthData), age(from.age), seed(from.seed), metamerCount(from.metamerCount), budCount(from.budCount)
{
	root = new TreeNode(*from.root);
	std::queue<TreeNode*> queue({ root });
	while (!queue.empty())
	{
		TreeNode* selected = queue.front();
		queue.pop();
		if (selected->nodeStatus != TreeNode::ALIVE)
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
	vec3 lateralDir = util::randomPerturbateVector(glm::normalize(bud.direction), growthData.lateralAngle, world->getWorldInfo().seed + seed + age + bud.id * 2 + 2);
	bud.lateralChild = new TreeNode(&bud, lastNodeId, bud.endPos(), lateralDir);
	budCount++;
	lastNodeId++;

	bud.nodeStatus = TreeNode::ALIVE;
	bud.createdAt = age;
	metamerCount++;
	maxOrder = glm::max(maxOrder, bud.order + 1);

	addShadows(*bud.mainChild);
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
	return world->getLightAt(bud.startPos);
}

void Tree::addNewShoots()
{
	addShootsRecursive(*root);
}

void Tree::shedBranchs()
{
	shedBranchsRecursive(*root);
}

void Tree::removeNode(TreeNode& node)
{
	if (node.nodeStatus != TreeNode::ALIVE)
		return;

	node.nodeStatus = TreeNode::DEAD;

	bool shouldRemove = node.order != 0 && node.parent->mainChild->id == node.id;

	if (node.parent)
	{
		if (node.parent->mainChild->id == node.id)
		{
			node.parent->mainChild = nullptr;
		}
		else if (node.parent->lateralChild->id == node.id)
		{
			node.parent->lateralChild = nullptr;
		}
		else
		{
			assert(0);
		}
	}
	else if (root->id == node.id)
	{
		root = nullptr;
	}

	std::queue<std::pair<TreeNode*, bool>> query({ std::make_pair(&node, shouldRemove) });
	while (!query.empty())
	{
		auto& [sel, shouldRem] = query.front();
		if (node.nodeStatus == TreeNode::ALIVE)
		{
			if (sel->mainChild)
				query.emplace(std::make_pair(sel->mainChild, true));
			if (sel->lateralChild)
				query.emplace(std::make_pair(sel->lateralChild, false));
			if (shouldRemove)
			{
				removeShadows(node);
			}
		}
		query.pop();
		delete& node;
	}



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
	if (node.nodeStatus == TreeNode::ALIVE)
	{
		vec3 start = node.startPos;
		vec3 end = node.endPos();
		std::cout << prefix << "Start: " << start.x << ", " << start.y << ", " << start.z << " End:" << end.x << ", " << end.y << ", " << end.z << " Light: " << node.light << std::endl;
		printTreeRecursive(*node.mainChild, prefix + " ");
		printTreeRecursive(*node.lateralChild, prefix + " ");
	}
}

/*void Tree::calculateShadows() const
{
	calculateShadowsRecursive(*root);
}*/

std::vector<TreeNode> Tree::AsNodeVector(bool includeBuds) const
{
	if (root->nodeStatus != TreeNode::ALIVE)
		return {};
	std::vector<TreeNode> nodes;
	std::queue<const TreeNode*> queue({ root });
	while (!queue.empty())
	{
		const TreeNode* selected = queue.front();
		queue.pop();
		if (selected->nodeStatus == TreeNode::ALIVE)
		{
			if (includeBuds || selected->mainChild->nodeStatus == TreeNode::ALIVE)
			{
				queue.push(selected->mainChild);
			}
			if (includeBuds || selected->lateralChild->nodeStatus == TreeNode::ALIVE)
			{
				queue.push(selected->lateralChild);
			}
		}
		nodes.push_back(*selected);
	}

	return nodes;
}

const std::vector<rb<Branch>>& Tree::getBranchs()
{
	if (!branchs.empty())
		return branchs;
	return recalculateBranchs(false);
}

const std::vector<rb<Branch>>& Tree::recalculateBranchs(bool clearCache)
{
	//cache old branchs and their leaves
	branchs.clear();
	if (clearCache)
	{
		cachedBranchs.clear();
	}
	if (root->nodeStatus != TreeNode::ALIVE)
		return branchs;
	std::stack<const TreeNode*> stack({ root });
	float lastOffset = 0.0f;
	vec3 lastPlaneNormal(1.0f, 0.0f, 0.0f);
	float length = 0.0f;
	branchs.reserve(cachedBranchs.size());

	while (!stack.empty())
	{
		const TreeNode* selected = stack.top();
		stack.pop();

		bool branchEnd = true;

		const TreeNode* weakChild = selected->weakerChild();
		if (weakChild != nullptr)
		{
			stack.push(weakChild);
		}
		const TreeNode* domChild = selected->dominantChild();
		if (domChild != nullptr)
		{
			stack.push(domChild);
			branchEnd = false;
		}

		if (branchEnd)
		{
			lastOffset = 0.0f;
			lastPlaneNormal = vec3(1.0f, 0.0f, 0.0f);
			length = 0.0f;
		}

		auto id = selected->id;
		Branch* curBranch;
		if (auto it = cachedBranchs.find(id); it != cachedBranchs.end())
		{
			curBranch = &cachedBranchs.at(id);
			curBranch->updateBranch(growthData.baseRadius, growthData.radiusN);
		}
		else
		{
			curBranch =
				&cachedBranchs.try_emplace(id, selected, growthData.baseRadius,
					growthData.radiusN, growthData.branchCurviness, length, lastPlaneNormal,
					lastOffset).first->second;
		}

		if (!branchEnd)
		{
			lastOffset = curBranch->offset;
			lastPlaneNormal = curBranch->bez.bezierPlaneNormal;
			length += curBranch->length;
		}
		branchs.push_back(curBranch);
	}
	generateLeaves();
	return branchs;
}

void Tree::generateLeaves()
{
	for (auto& [_, branch] : cachedBranchs)
	{
		//TODO cache these too
		branch.generateLeaves(growthData.leafMaxChildCount, growthData.leafMinOrder, growthData.leafDensity, growthData.leafSizeMultiplier);
	}
}

Tree::~Tree()
{
	OnDestroy.dispatch({});
	if (!root)
		return;
	std::queue<TreeNode*> queue({ root });
	while (!queue.empty())
	{
		TreeNode* selected = queue.front();
		queue.pop();
		if (selected->nodeStatus == TreeNode::ALIVE)
		{
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
	if (node.nodeStatus == TreeNode::BUD)
	{
		if (!world->isOutOfBounds(node.startPos))
		{
			node.light = lightAtBud(node);
		}
		else
		{
			node.light = 0.0f;
		}
	}
	else if (node.nodeStatus == TreeNode::ALIVE)
	{
		node.light = accumulateLightRecursive(*node.mainChild) + accumulateLightRecursive(*node.lateralChild);
	}
	else
	{
		node.light = 0.0f;
	}
	return node.light;
}

void Tree::distributeVigorRecursive(TreeNode& node)
{
	if (node.nodeStatus != TreeNode::ALIVE)
		return;
	float apicalControl = growthData.apicalControl;
	float mainV = 0.0f;
	float lateralV = 0.0f;
	if (node.mainChild->nodeStatus != TreeNode::DEAD)
	{
		mainV = apicalControl * node.mainChild->light;
	}
	if (node.lateralChild->nodeStatus != TreeNode::DEAD)
	{
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
	if (node.nodeStatus == TreeNode::ALIVE)
	{
		addShootsRecursive(*node.mainChild);
		addShootsRecursive(*node.lateralChild);
		return;
	}
	else if (node.nodeStatus == TreeNode::DEAD)
	{
		return;
	}

	float vigor = node.vigor;
	int vigorFloored = static_cast<int>(glm::floor(vigor));

	float metamerLength = growthData.baseLength * vigor / glm::floor(vigor);

	if (world->isOutOfBounds(node.startPos))
		return;

	vec3 optimal = world->getOptimalDirection(node.startPos);

	vec3 direction = node.direction;

	TreeNode* current = &node;


	for (int i = 0; i < vigorFloored; i++)
	{
		if (world->isOutOfBounds(current->startPos))
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
	if (node.nodeStatus != TreeNode::ALIVE)
		return;

	float p = node.vigor - growthData.shedMultiplier *
		glm::pow(glm::pow(node.childCount, 1.0f / growthData.radiusN), growthData.shedExp);

	if (age - node.createdAt < 4 || p >= 0)
	{
		shedBranchsRecursive(*node.mainChild);
		shedBranchsRecursive(*node.lateralChild);
		return;
	}

	node.nodeStatus = TreeNode::DEAD;
	cachedBranchs.erase(node.id);
	if (node.order != 0 && node.parent->mainChild->id == node.id)
	{
		removeShadows(node);
	}
	std::queue<TreeNode*> query({ node.mainChild, node.lateralChild });
	while (!query.empty())
	{
		TreeNode* sel = query.front();
		cachedBranchs.erase(sel->id);
		if (node.nodeStatus == TreeNode::ALIVE)
		{
			query.emplace(sel->mainChild);
			if (sel->mainChild->nodeStatus != TreeNode::DEAD)
				removeShadows(*sel->mainChild);

			query.emplace(sel->lateralChild);
		}
		query.pop();
		delete sel;
	}
}

float Tree::calculateChildCountRecursive(TreeNode& node)
{
	if (node.nodeStatus == TreeNode::BUD)
	{
		return 1;
	}
	else if (node.nodeStatus == TreeNode::DEAD)
	{
		return 1;
	}
	else
	{
		uint32 newChildCount = calculateChildCountRecursive(*node.mainChild) + calculateChildCountRecursive(*node.lateralChild);
		node.childCount = glm::max(node.childCount, newChildCount);
		return node.childCount;
	}
}

void Tree::removeShadows(const TreeNode& node) const
{
	world->castShadows(node.startPos, false);
}
void Tree::addShadows(TreeNode& node)
{
	world->castShadows(node.startPos, true);
}
}