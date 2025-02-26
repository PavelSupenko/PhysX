#include "NvBlastExtAuthoringMeshCleaner.h"
#include "NvBlastExtAuthoringFractureTool.h"
#include "NvBlastExtAuthoringBondGenerator.h"
#include "NvBlastExtAuthoring.h"
#include "SimpleRandomGenerator.h"
#include "NvBlastExtUnity.h"
#include "NvBlastPreprocessorInternal.h" // for log macros
#include "BoundingBoxConvexMeshBuilder.h"

using namespace Nv::Blast;

Mesh* NvBlastExtUnityCreateMesh(const NvcVec3* position, const NvcVec3* normals, const NvcVec2* uv, uint32_t verticesCount, const uint32_t* triangleIndices, uint32_t indicesCount)
{
	Mesh* mesh = NvBlastExtAuthoringCreateMesh(position, normals, uv, verticesCount, triangleIndices, indicesCount);
	return mesh;
}

void NvBlastExtUnityReleaseMesh(Mesh* mesh)
{
    mesh->release();
}

void NvBlastExtUnitySetMaterialId(Mesh* mesh, const int32_t *materialIds)
{
	mesh->setMaterialId(materialIds);
}

void NvBlastExtUnitySetSmoothingGroup(Mesh* mesh, const int32_t *smoothingGroups)
{
	mesh->setSmoothingGroup(smoothingGroups);
}

uint32_t NvBlastExtUnityGetVerticesCount(const Mesh* mesh)
{
	return mesh->getVerticesCount();
}

const Vertex* NvBlastExtUnityGetVertices(const Mesh* mesh)
{
	return mesh->getVertices();
}

uint32_t NvBlastExtUnityGetFacetCount(const Mesh* mesh)
{
	return mesh->getFacetCount();
}

const Facet* NvBlastExtUnityGetFacets(const Mesh* mesh)
{
	return mesh->getFacetsBuffer();
}

uint32_t NvBlastExtUnityGetEdgesCount(const Mesh* mesh)
{
	return mesh->getEdgesCount();
}

const Edge* NvBlastExtUnityGetEdges(const Mesh* mesh)
{
	return mesh->getEdges();
}

// Original mesh will be disposed!
Mesh* NvBlastExtUnityCleanMesh(Mesh* mesh)
{
    MeshCleaner* clr = NvBlastExtAuthoringCreateMeshCleaner();
	Mesh* nmesh;
	nmesh = clr->cleanMesh(mesh);
	clr->release();
	mesh->release();
	return nmesh;
}

AuthoringResult* NvBlastExtUnityVoronoiFractureMesh(Mesh *mesh, u_int32_t cellsCount, uint32_t aggregateMaxCount, NvBlastLog logFn)
{
	FractureTool* fTool = NvBlastExtAuthoringCreateFractureTool();
	Mesh const* const meshes[1] = { mesh };
	int32_t ids[1] = { 0 };
	fTool->setSourceMeshes(meshes, 1, ids);

	SimpleRandomGenerator rng;
	rng.seed(0);

	VoronoiSitesGenerator* voronoiSitesGenerator = NvBlastExtAuthoringCreateVoronoiSitesGenerator(mesh, &rng);
	if (voronoiSitesGenerator == nullptr)
	{
        NVBLASTLL_LOG_ERROR(logFn, "Failed to create Voronoi sites generator");
		return nullptr;
	}

	// case 'v':
	NVBLASTLL_LOG_DEBUG(logFn, "Fracturing with Voronoi...");
	voronoiSitesGenerator->uniformlyGenerateSitesInMesh(cellsCount);
	const NvcVec3* sites = nullptr;
	uint32_t sitesCount = voronoiSitesGenerator->getVoronoiSites(sites);
	if (fTool->voronoiFracturing(0, sitesCount, sites, false) != 0)
	{
		NVBLASTLL_LOG_ERROR(logFn, "Failed to fracture with Voronoi");
		return nullptr;
	}

	NVBLASTLL_LOG_DEBUG(logFn, "Releasing sites generator and mesh...");
	voronoiSitesGenerator->release();
	mesh->release();

	NVBLASTLL_LOG_DEBUG(logFn, "Creating bonds...");
	ConvexMeshBuilder* collisionBuilder = new BoundingBoxConvexMeshBuilder();
	BlastBondGenerator* bondGenerator   = NvBlastExtAuthoringCreateBondGenerator(collisionBuilder);
	
	ConvexDecompositionParams collisionParameter;
	collisionParameter.maximumNumberOfHulls = aggregateMaxCount > 0 ? aggregateMaxCount : 1;
	collisionParameter.voxelGridResolution = 0;

	NVBLASTLL_LOG_DEBUG(logFn, "Fracturing...");
	AuthoringResult* result = NvBlastExtAuthoringProcessFracture(*fTool, *bondGenerator, *collisionBuilder, collisionParameter);

	bondGenerator->release();
	fTool->release();

	bool fbxCollision = false; // Add collision geometry to FBX file
	if (!fbxCollision)
	{
		NvBlastExtAuthoringReleaseAuthoringResultCollision(*collisionBuilder, result);
	}

	NVBLASTLL_LOG_DEBUG(logFn, "Success");
	return result;
}

uint32_t NvBlastExtUnityGetFractureChunksCount(const AuthoringResult& aResult)
{
	uint32_t meshCount = aResult.chunkCount;
	return meshCount;
}

Mesh** NvBlastExtUnityCreateMeshes(const AuthoringResult& aResult)
{
    // Проверка корректности входных данных
    if (aResult.chunkCount == 0)
        return nullptr;

    // Количество создаваемых мешей равно количеству чанков
    uint32_t meshCount = aResult.chunkCount;
    Mesh** meshes = new Mesh*[meshCount];

    // Для каждого чанка создаём отдельный Mesh
    for (uint32_t i = 0; i < meshCount; ++i)
    {
        // Каждый чанк описывается диапазоном треугольников:
        // [geometryOffset[i], geometryOffset[i+1])
        uint32_t start = aResult.geometryOffset[i];
        uint32_t end   = aResult.geometryOffset[i + 1];
        uint32_t triangleCount = end - start;
        uint32_t vertexCount   = triangleCount * 3; // 3 вершины на треугольник

        // Выделяем временные массивы для вершин, нормалей, UV и индексов
        NvcVec3* positions = new NvcVec3[vertexCount];
        NvcVec3* normals   = new NvcVec3[vertexCount];
        NvcVec2* uvs       = new NvcVec2[vertexCount];
        uint32_t* indices  = new uint32_t[vertexCount];

        // Заполняем массивы данными из треугольников чанка
        uint32_t v = 0;
        for (uint32_t t = 0; t < triangleCount; ++t)
        {
            const Triangle& tri = aResult.geometry[start + t];
            
            // Вершина A
            positions[v] = tri.a.p;
            normals[v]   = tri.a.n;
            uvs[v]       = tri.a.uv[0];
            indices[v]   = v;
            ++v;
            
            // Вершина B
            positions[v] = tri.b.p;
            normals[v]   = tri.b.n;
            uvs[v]       = tri.b.uv[0];
            indices[v]   = v;
            ++v;
            
            // Вершина C
            positions[v] = tri.c.p;
            normals[v]   = tri.c.n;
            uvs[v]       = tri.c.uv[0];
            indices[v]   = v;
            ++v;
        }

        // Создаём Mesh с помощью предоставленной API-функции
        Mesh* mesh = NvBlastExtAuthoringCreateMesh(positions, normals, uvs, vertexCount, indices, vertexCount);
        meshes[i] = mesh;

        // Освобождаем временные массивы, так как API, как правило, копирует данные
        delete[] positions;
        delete[] normals;
        delete[] uvs;
        delete[] indices;
    }

    return meshes;
}
