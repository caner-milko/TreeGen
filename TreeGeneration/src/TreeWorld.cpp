#include "TreeWorld.h"
#include "./Util.hpp"
#include <iostream>
TreeWorld::TreeWorld(const BBox& worldBoundingBox, float cellSize, uint32 seed) : seed(seed)
{
	resizeShadowGrid(ivec3(glm::ceil((worldBoundingBox.max - worldBoundingBox.min) / cellSize)), worldBoundingBox.min, cellSize);
}

TreeWorld::TreeWorld(ivec3 worldSize, vec3 leftBottomCorner, float cellSize, uint32 seed) : seed(seed)
{
	resizeShadowGrid(worldSize, leftBottomCorner, cellSize);
}

void TreeWorld::resizeShadowGrid(ivec3 worldSize, vec3 leftBottomCorner, float cellSize)
{
	this->worldSize = worldSize;
	this->leftBottomCorner = leftBottomCorner;
	this->cellSize = cellSize;
	this->shadowGrid = std::vector(worldSize.x * worldSize.y * worldSize.z, ShadowCell());
	std::fill(shadowGrid.begin(), shadowGrid.end(), ShadowCell{ 0.0f });
	std::cout << "World Size: " << shadowGrid.size() << " Bytes: " << shadowGrid.size() * sizeof(ShadowCell) << std::endl;
}

/*void TreeWorld::calculateShadows()
{
	std::fill(shadowGrid.begin(), shadowGrid.end(), ShadowCell{ 0.0f });
	for (auto& tree : trees) {
		//tree->calculateShadows();
	}
}*/

Tree* TreeWorld::createTree(vec3 position, TreeGrowthData growthData)
{
	treeCount++;
	return trees.emplace_back(std::make_unique<Tree>(*this, trees.size(), position, growthData, trees.size())).get();//util::hash(static_cast<uint32>(trees.size())))).get();
}

void TreeWorld::removeTree(Tree& tree)
{
	for (int i = 0; i < trees.size(); i++) {
		if (*trees[i] == tree)
		{
			trees.erase(trees.begin() + i);
		}
	}
}

float TreeWorld::getLightAt(const vec3& position, float a, float fullExposure)
{
	ivec3 relScaledPos = coordinateToCell(position);
	if (relScaledPos.x < 0 || relScaledPos.x >= worldSize.x || relScaledPos.y < 0 || relScaledPos.y >= worldSize.y || relScaledPos.z < 0 || relScaledPos.z >= worldSize.z)
		return 0.0f;
	return glm::max(fullExposure - getCellAt(relScaledPos).shadow + a, 0.0f);
}

vec3 TreeWorld::getOptimalDirection(const vec3& position)
{
	if (isOutOfBounds(position)) {
		return vec3(0.0f, 1.0f, 0.0f);
	}
	ivec3 relScaledPos = coordinateToCell(position);

	ivec3 negX = relScaledPos - ivec3(1, 0, 0);
	ivec3 posX = relScaledPos + ivec3(1, 0, 0);

	if (negX.x < 0)
		negX = relScaledPos;

	if (posX.x >= worldSize.x)
		posX = relScaledPos;

	float xGrad = (getCellAt(posX).shadow - getCellAt(negX).shadow) / static_cast<float>(posX.x - negX.x);


	ivec3 negY = relScaledPos - ivec3(0, 1, 0);
	ivec3 posY = relScaledPos + ivec3(0, 1, 0);


	if (negY.y < 0)
		negY = relScaledPos;

	if (posY.y >= worldSize.y)
		posY = relScaledPos;

	float yGrad = (getCellAt(posY).shadow - getCellAt(negY).shadow) / static_cast<float>(posY.y - negY.y);

	ivec3 negZ = relScaledPos - ivec3(0, 0, 1);
	ivec3 posZ = relScaledPos + ivec3(0, 0, 1);


	if (negZ.z < 0)
		negZ = relScaledPos;

	if (posZ.z >= worldSize.z)
		posZ = relScaledPos;

	float zGrad = (getCellAt(posZ).shadow - getCellAt(negZ).shadow) / static_cast<float>(posZ.z - negZ.z);

	vec3 gradient = vec3(xGrad, yGrad, zGrad);
	if (glm::length(gradient) == 0.0f)
		return vec3(0.0f, 1.0f, 0.0f);

	return -glm::normalize(vec3(xGrad, yGrad, zGrad));

	/*float cos = glm::cos(perceptionAngle);
	float lowest = std::numeric_limits<float>().max();
	vec3 selDir = direction;
	int sameLowest = 0;
	ivec3 relScaledPos = coordinateToCell(position);
	float scaledPerception = perceptionRadius / cellSize;
	int perceptionFloor = static_cast<int>(glm::floor(scaledPerception));
	int perceptionCeil = static_cast<int>(glm::ceil(scaledPerception));
	for (int i = glm::max(relScaledPos.x - perceptionFloor, 0); i <= glm::min(relScaledPos.x + perceptionCeil, worldSize.x - 1); i++) {
		for (int j = glm::max(relScaledPos.y - perceptionFloor, 0); j <= glm::min(relScaledPos.y + perceptionCeil, worldSize.y - 1); j++) {
			for (int k = glm::max(relScaledPos.z - perceptionFloor, 0); k <= glm::min(relScaledPos.z + perceptionCeil, worldSize.z - 1); k++) {

				ivec3 cellPos = ivec3(i, j, k);
				vec3 cellCoord = cellToCoordinate(cellPos);
				vec3 dif = cellCoord - position;
				vec3 normalizedDif = glm::normalize(dif);
				float dot = glm::dot(direction, normalizedDif);
				if (dot < cos)
					continue;
				if (glm::length(dif) > perceptionRadius)
					continue;
				ShadowCell& cell = getCellAt(cellPos);
				float shadowAt = cell.shadow;
				if (shadowAt < lowest) {
					lowest = shadowAt;
					selDir = normalizedDif;
					sameLowest = 1;
				}
				if (shadowAt == lowest) {
					sameLowest++;
					selDir += normalizedDif;
				}
			}
		}
	}
	return selDir / (float)sameLowest;*/

}

void TreeWorld::castShadows(const vec3& pos, int pyramidHeight, float a, float b, bool addShadows)
{
	ivec3 relScaledPos = coordinateToCell(pos);
	for (int q = 0; q <= pyramidHeight; q++) {
		int j = relScaledPos.y - q;
		if (j < 0)
			break;
		if (j >= worldSize.y)
			continue;
		for (int i = glm::max(relScaledPos.x - q, 0); i <= glm::min(relScaledPos.x + q, worldSize.x - 1); i++) {
			for (int k = glm::max(relScaledPos.z - q, 0); k <= glm::min(relScaledPos.z + q, worldSize.z - 1); k++) {
				float s = a * glm::pow(b, -(q / 2.0f));
				addShadowTo(ivec3(i, j, k), s * (float(addShadows) * 2.0f - 1.0f));
			}
		}
	}
}

ivec3 TreeWorld::coordinateToCell(const vec3& pos) const
{
	vec3 relPos = (pos - leftBottomCorner) / cellSize;
	int x = static_cast<int>(glm::round(relPos.x));
	int y = static_cast<int>(glm::round(relPos.y));
	int z = static_cast<int>(glm::round(relPos.z));
	return ivec3(x, y, z);
}

vec3 TreeWorld::cellToCoordinate(const ivec3& cell) const
{
	return vec3(cell) * cellSize + leftBottomCorner;
}

int TreeWorld::cellToIndex(const ivec3& cell) const
{
	return worldSize.x * worldSize.y * cell.z + worldSize.x * cell.y + cell.x;
}

BBox TreeWorld::getBBox() const
{
	return BBox(leftBottomCorner, leftBottomCorner + vec3(worldSize) * cellSize);
}

bool TreeWorld::isOutOfBounds(const vec3& pos) const
{
	ivec3 cell = coordinateToCell(pos);
	return !(cell.x >= 0 && cell.x < worldSize.x&& cell.y >= 0 && cell.y < worldSize.y&& cell.z >= 0 && cell.z < worldSize.z);
}

std::vector<vec4> TreeWorld::renderShadowCells(const vec3& camPos, const vec3& viewDir, float fov, float visibilityRadius) const
{
	ivec3 camToCell = coordinateToCell(camPos);

	float visibilityScaled = visibilityRadius / cellSize;

	int visibilityScaledFloored = static_cast<int>(glm::floor(visibilityScaled));
	int visibilityScaledCeiled = static_cast<int>(glm::ceil(visibilityScaled));

	float fovCos = glm::cos(fov);
	std::vector<vec4> cells;
	for (int i = glm::max(camToCell.x - visibilityScaledFloored, 0); i <= glm::min(camToCell.x + visibilityScaledCeiled, worldSize.x - 1); i++) {
		for (int j = glm::max(camToCell.y - visibilityScaledFloored, 0); j <= glm::min(camToCell.y + visibilityScaledCeiled, worldSize.y - 1); j++) {
			for (int k = glm::max(camToCell.z - visibilityScaledFloored, 0); k <= glm::min(camToCell.z + visibilityScaledCeiled, worldSize.z - 1); k++) {

				ivec3 cellPos = ivec3(i, j, k);
				vec3 cellCoord = cellToCoordinate(cellPos);
				vec3 dif = cellCoord - camPos;
				vec3 normalizedDif = glm::normalize(dif);
				float dot = glm::dot(viewDir, normalizedDif);
				if (dot < fovCos)
					continue;
				if (glm::length(dif) > visibilityRadius)
					continue;
				const ShadowCell& cell = getCellAt(cellPos);
				float shadowAt = cell.shadow;
				if (shadowAt == 0.0f) {
					continue;
				}
				cells.emplace_back(cellCoord, shadowAt);
			}
		}
	}
	return cells;
}


ShadowCell& TreeWorld::getCellAt(const ivec3& cell)
{
	return shadowGrid[cellToIndex(cell)];
}

const ShadowCell& TreeWorld::getCellAt(const ivec3& cell) const
{
	return shadowGrid[cellToIndex(cell)];
}

void TreeWorld::addShadowTo(const ivec3& cell, float amount)
{
	getCellAt(cell).shadow += amount;
}
