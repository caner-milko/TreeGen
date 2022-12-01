#include "TreeWorld.h"
#include "./Util.hpp"
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
}

void TreeWorld::calculateShadows()
{
	std::fill(shadowGrid.begin(), shadowGrid.end(), ShadowCell{ 0.0f });
	for (auto& tree : trees) {
		tree->calculateShadows();
	}
}

Tree* TreeWorld::createTree(vec3 position)
{
	return trees.emplace_back(std::make_unique<Tree>(*this, position, util::hash(static_cast<uint32>(trees.size())))).get();
}

float TreeWorld::getLightAt(const vec3& position, float a, float fullExposure)
{
	ivec3 relScaledPos = coordinateToCell(position);
	if (relScaledPos.x < 0 || relScaledPos.x >= worldSize.x || relScaledPos.y < 0 || relScaledPos.y >= worldSize.y || relScaledPos.z < 0 || relScaledPos.z >= worldSize.z)
		return 0.0f;
	return glm::max(fullExposure - getCellAt(relScaledPos).shadow + a, 0.0f);
}

vec3 TreeWorld::getOptimalDirection(const vec3& position, const vec3& direction, float perceptionRadius, float perceptionAngle)
{
	float cos = glm::cos(perceptionAngle);
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
	return selDir / (float)sameLowest;

}

void TreeWorld::castShadows(const vec3& pos, int pyramidHeight, float a, float b)
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
				float s = a * glm::pow(b, -q);
				addShadowTo(ivec3(i, j, k), s);
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
	return vec3(x, y, z);
}

vec3 TreeWorld::cellToCoordinate(const ivec3& cell) const
{
	return vec3(cell) * cellSize + leftBottomCorner;
}

int TreeWorld::cellToIndex(const ivec3& cell) const
{
	return worldSize.x * worldSize.y * cell.z + worldSize.x * cell.y + cell.x;
}

std::vector<std::tuple<vec3, float>> TreeWorld::renderShadowCells(const vec3& camPos, const vec3& viewDir, float fov, float visibilityRadius) const
{
	ivec3 camToCell = coordinateToCell(camPos);

	float visibilityScaled = visibilityRadius / cellSize;

	int visibilityScaledFloored = glm::floor(visibilityScaled);
	int visibilityScaledCeiled = glm::ceil(visibilityScaled);

	float fovCos = glm::cos(fov);
	std::vector<std::tuple<vec3, float>> cells;
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
				cells.emplace_back < std::tuple<vec3, float>>({ cellCoord, shadowAt });
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
