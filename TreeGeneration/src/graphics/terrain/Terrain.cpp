#include "Terrain.h"
namespace tgen::graphics::terrain
{
Terrain::Terrain(const TerrainData& data) : data(data)
{
}

void Terrain::generateMesh(std::vector<TerrainVertex>* vertices, std::vector<uint32>* indices) const
{
	int verticesSize = (data.gridSize.x + 1) * (data.gridSize.y + 1);
	*vertices = std::vector<TerrainVertex>();
	vertices->reserve(verticesSize);
	int indicesSize = data.gridSize.x * data.gridSize.y * 2 * 3;
	*indices = std::vector<uint32>();
	indices->reserve(indicesSize);


	for (int32 y = 0; y <= data.gridSize.y; y++)
	{
		for (int32 x = 0; x <= data.gridSize.x; x++)
		{
			TerrainVertex vertex;
			vec2 uv = cellSize() * vec2(x, y);
			vertex.pos = vec3(uv.x, valueAt(uv), uv.y);
			vertex.normal = normalAt(vec2(x, y));
			vertices->emplace_back(vertex);
		}
	}
	for (int32 x = 0; x < data.gridSize.x; x++)
	{
		for (int32 y = 0; y < data.gridSize.y; y++)
		{
			//first triangle
			indices->emplace_back(vertexIndex(ivec2(x, y + 1)));
			indices->emplace_back(vertexIndex(ivec2(x + 1, y)));
			indices->emplace_back(vertexIndex(ivec2(x, y)));
			//second triangle
			indices->emplace_back(vertexIndex(ivec2(x, y + 1)));
			indices->emplace_back(vertexIndex(ivec2(x + 1, y + 1)));
			indices->emplace_back(vertexIndex(ivec2(x + 1, y)));
		}
	}
	int indexSize = indices->size();

}

vec2 Terrain::worldPosToUv(vec2 worldPos) const
{
	vec2 center2D = vec2(data.center.x, data.center.z);
	vec2 transformed = worldPos - center2D + data.size / 2.0f;
	transformed = transformed / data.size;
	return glm::clamp(transformed, 0.0f, 1.0f);
}

float Terrain::heightAtWorldPos(vec3 worldPos) const
{
	return heightAt(worldPosToUv(vec2(worldPos.x, worldPos.z)));
}

float Terrain::heightAt(vec2 uv) const
{
	return data.center.y + glm::mix(data.minHeight, data.maxHeight, valueAt(uv));
}

float Terrain::valueAt(vec2 uv) const
{
	return data.heightMap->getRGB<1>(uv).x / 256.0f;
}

vec3 Terrain::normalAt(ivec2 vertex) const
{
	vec2 nX = vec2(vertex) + vec2(-1.0f * (vertex.x > 0), 0);
	vec2 nY = vec2(vertex) + vec2(0, -1.0f * (vertex.y > 0));
	vec2 pX = vec2(vertex) + vec2(1.0f * (vertex.x <= data.gridSize.y), 0);
	vec2 pY = vec2(vertex) + vec2(0, 1.0f * (vertex.y <= data.gridSize.y));

	vec2 cells = cellSize();

	vec2 nXuv = cells * vec2(nX);
	vec2 nYuv = cells * vec2(nY);
	vec2 pXuv = cells * vec2(pX);
	vec2 pYuv = cells * vec2(pY);

	float nXh = valueAt(nXuv);
	float nYh = valueAt(nYuv);
	float pXh = valueAt(pXuv);
	float pYh = valueAt(pYuv);

	vec2 posuv = cells * vec2(vertex);
	vec3 pos = vec3(posuv.x, valueAt(posuv), posuv.y);

	float xGrad = (pXh - nXh) / cells.x / (pX.x - nX.x);
	float yGrad = (pYh - nYh) / cells.y / (pY.y - nY.y);

	return glm::normalize(vec3(-xGrad, 2.0f, -yGrad));

	//vec3 nX3 = vec3(nXuv.x / cells.x, nXh, nXuv.y / cells.y);// / vec3(cells.x, 1.0f, cells.y);
	//vec3 nY3 = vec3(nYuv.x / cells.x, nYh, nYuv.y / cells.y);// / vec3(cells.x, 1.0f, cells.y);
	//vec3 pX3 = vec3(pXuv.x / cells.x, pXh, pXuv.y / cells.y);// / vec3(cells.x, 1.0f, cells.y);
	//vec3 pY3 = vec3(pYuv.x / cells.x, pYh, pYuv.y / cells.y);// / vec3(cells.x, 1.0f, cells.y);

	//return glm::normalize((pY3 - nY3), (pX3 - nX3)));

}

Camera Terrain::getTerrainCamera() const
{
	Camera cam;
	cam.projection = vec4(-data.size.x * 0.5f, data.size.x * 0.5f, -data.size.y * 0.5f, data.size.y * 0.5f);
	cam.nearPlane = 1.0f;
	cam.farPlane = 50.0f;
	cam.cameraPosition = data.center + vec3(0, glm::max(10.0f, data.maxHeight + 15.0f), 0);
	cam.dir = vec3(0.0f, -1.0f, 0.0f);
	cam.up = vec3(0.0f, 0.0f, -1.0f);
	return cam;
}

}