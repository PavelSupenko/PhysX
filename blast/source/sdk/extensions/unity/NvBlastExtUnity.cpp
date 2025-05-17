#include "NvBlastExtAuthoringMeshCleaner.h"
#include "NvBlastExtAuthoringFractureTool.h"
#include "NvBlastExtAuthoringBondGenerator.h"
#include "NvBlastExtAuthoring.h"
#include "SimpleRandomGenerator.h"
#include "NvBlastExtUnity.h"
#include "NvBlastPreprocessorInternal.h" // for log macros
#include "BoundingBoxConvexMeshBuilder.h"

#include "VoronoiFracturer.h"
#include "IslandsFracturer.h"
#include "ClusteredVoronoiFracturer.h"
#include "SlicingFracturer.h"
#include "PlaneCutFracturer.h"
#include "CutOutFracturer.h"

#include <sstream>

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

Mesh* NvBlastExtUnityCleanMesh(Mesh* mesh)
{
    MeshCleaner* clr = NvBlastExtAuthoringCreateMeshCleaner();
	Mesh* nmesh;
	nmesh = clr->cleanMesh(mesh);
	clr->release();

	// Original mesh disposing
	mesh->release();
	return nmesh;
}

Fracturer* NvBlastExtUnityCreateVoronoiFracturer(VoronoiConfiguration settings)
{
	return new VoronoiFracturer(settings);
}

Fracturer* NvBlastExtUnityCreateClusteredVoronoiFracturer(ClusteredVoronoiConfiguration settings)
{
	return new ClusteredVoronoiFracturer(settings);
}

Fracturer* NvBlastExtUnityCreateSlicingFracturer(SlicingConfiguration settings)
{
	return new SlicingFracturer(settings);
}

Fracturer* NvBlastExtUnityCreateIslandsFracturer()
{
	return new IslandsFracturer();
}

Fracturer* NvBlastExtUnityCreatePlaneCutFracturer(PlaneCutConfiguration settings)
{
	return new PlaneCutFracturer(settings);
}

Fracturer* NvBlastExtUnityCreateCutOutFracturer(CutOutConfiguration settings)
{
	return new CutOutFracturer(settings);
}

AuthoringResult* NvBlastExtUnityFractureMesh(Mesh *mesh, uint32_t aggregateMaxCount, Fracturer* fracturer, NvBlastLog logFn)
{
	Mesh* meshes[] = { mesh };
	int32_t ids[] = { 0 };
	return NvBlastExtUnityFractureMeshes(meshes, 1, ids, aggregateMaxCount, fracturer, logFn);
}

AuthoringResult* NvBlastExtUnityFractureMeshes(Mesh **meshes, uint32_t meshesSize, const int32_t *ids, uint32_t aggregateMaxCount, Fracturer* fracturer, NvBlastLog logFn)
{
	std::ostringstream oss;
	oss << "Fracturing " << meshesSize << " meshes...";
	NVBLASTLL_LOG_DEBUG(logFn, oss.str().c_str());

	FractureTool* fTool = NvBlastExtAuthoringCreateFractureTool();
	ConvexMeshBuilder* collisionBuilder = new BoundingBoxConvexMeshBuilder();
	BlastBondGenerator* bondGenerator = NvBlastExtAuthoringCreateBondGenerator(collisionBuilder);

	ConvexDecompositionParams collisionParameter;
	collisionParameter.maximumNumberOfHulls = aggregateMaxCount > 0 ? aggregateMaxCount : 1;
	collisionParameter.voxelGridResolution = 0;

	SimpleRandomGenerator rng;
	rng.seed(0);

	for (uint32_t i = 0; i < meshesSize; ++i)
	{
		Mesh* mesh = meshes[i];
		u_int32_t id = ids[i];

		if (mesh == nullptr)
		{
			std::ostringstream oss;
			oss << "Mesh with id: " << id << " is NULL";
			NVBLASTLL_LOG_DEBUG(logFn, oss.str().c_str());
		}
		else
		{
			std::ostringstream oss;
			oss << "Mesh with id: " << id << " is not NULL";

			bool isMeshValid = mesh->isValid();

			if (isMeshValid)
				oss << " and is valid";
			else
				oss << " and is not valid";

			NVBLASTLL_LOG_DEBUG(logFn, oss.str().c_str());
		}
	}

	fTool->setSourceMeshes(meshes, meshesSize, ids);
	NVBLASTLL_LOG_DEBUG(logFn, "Meshes set into fracture tool");

	for (uint32_t i = 0; i < meshesSize; ++i)
	{
		Mesh* mesh = meshes[i];
		u_int32_t id = ids[i];

		if (mesh == nullptr)
		{
			NVBLASTLL_LOG_ERROR(logFn, "Mesh is null");
			continue;
		}

		VoronoiSitesGenerator* voronoiSitesGenerator = NvBlastExtAuthoringCreateVoronoiSitesGenerator(mesh, &rng);
		if (voronoiSitesGenerator == nullptr)
		{
			NVBLASTLL_LOG_ERROR(logFn, "Failed to create Voronoi sites generator");
			return nullptr;
		}
		else
		{
			NVBLASTLL_LOG_DEBUG(logFn, "Voronoi sites generator created");
		}

		if (!fracturer->fracture(fTool, voronoiSitesGenerator, &rng, id, logFn))
		{
			return nullptr;
			NVBLASTLL_LOG_ERROR(logFn, "Failed to fracture mesh");
		}
	
		NVBLASTLL_LOG_DEBUG(logFn, "Releasing sites generator and mesh...");
		voronoiSitesGenerator->release();
		mesh->release();
	}

	NVBLASTLL_LOG_DEBUG(logFn, "Fracturing...");
	AuthoringResult* result = NvBlastExtAuthoringProcessFracture(*fTool, *bondGenerator, *collisionBuilder, collisionParameter);

	bondGenerator->release();
	collisionBuilder->release();
	fTool->release();

	// bool fbxCollision = false; // Add collision geometry to FBX file
	// if (!fbxCollision)
	// {
	// 	NvBlastExtAuthoringReleaseAuthoringResultCollision(*collisionBuilder, result);
	// }

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
