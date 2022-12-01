#pragma once
#include "TreeNode.h"
#include "Branch.h"
#include "Leaf.h"
#include <unordered_map>
#include <glm/gtc/constants.hpp>

#include <string>
struct TreeWorld;

struct TreeGrowthData
{
	float apicalControl = 0.5f;
	float baseLength = .25f;

	float baseRadius = 0.02f;
	float radiusN = 2.5f;
	//bud fate
	float perceptionRadius = 5.0;
	float perceptionAngle = glm::radians(45.0f);
	//lateral
	float lateralAngle = glm::radians(45.0f);
	//
	vec3 tropism = vec3(0.0f, -1.0f, 0.0f);
	//default, optimal, tropism
	vec3 directionWeights = glm::normalize(vec3(0.6f, 0.4f, 0.1f));
	//shadows
	float fullExposure = 4.0f;
	int pyramidHeight = 5;
	float a = 0.8f;
	//b > 1
	float b = 1.3f;

	//leaf params
	float leafMaxWidth = 0.2f;
	float leafDensity = 1.0f;

};

struct Tree
{
public:
	TreeWorld& world;
	TreeGrowthData growthData{};
	uint32 age = 0;
	uint32 seed = 0;
	TreeNode* root;

	int metamerCount = 0;
	int budCount = 0;

	Tree(TreeWorld& world, vec3 position, uint32 seed);
	Tree(const Tree& from);

	void init();

	void budToMetamer(TreeNode& bud);

	void startGrow();

	float accumulateLight();

	void distributeVigor();

	float lightAtBud(TreeNode& bud);

	void addNewShoots();

	void calculateRadiuses();

	void endGrow();

	void printTreeRecursive(TreeNode& node, const std::string& prefix) const;

	void calculateShadows() const;

	std::vector<TreeNode> AsNodeVector(bool includeBuds) const;



	const std::vector<Branch>& getBranchs();

	const std::vector<Branch>& recalculateBranchs();

	void generateLeaves();

	~Tree();

private:

	std::vector<Branch> branchs;
	float accumulateLightRecursive(TreeNode& node);
	void distributeVigorRecursive(TreeNode& node);
	void addShootsRecursive(TreeNode& node);
	float calculateRadiusRecursive(TreeNode& node);
	void calculateShadowsRecursive(TreeNode& node) const;
};
