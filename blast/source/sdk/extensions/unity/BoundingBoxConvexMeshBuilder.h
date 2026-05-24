#include "NvBlastExtAuthoring.h"
#include "NvBlastExtAuthoringConvexMeshBuilder.h"

#include <algorithm>
#include <cstdint>
#include <new>

namespace Nv
{
namespace Blast
{

class BoundingBoxConvexMeshBuilder : public ConvexMeshBuilder
{
    public:
    // Деструктор можно оставить виртуальным, если потребуется
    virtual ~BoundingBoxConvexMeshBuilder() {}

    // Освобождение памяти объекта билдера
    virtual void release() override {
        delete this;
    }

    // Создание CollisionHull, представляющего bounding box исходного набора вершин
    virtual CollisionHull* buildCollisionGeometry(uint32_t verticesCount, const NvcVec3* vertexData) override {
        if (verticesCount == 0 || vertexData == nullptr)
            return nullptr;

        // Вычисляем минимальные и максимальные координаты
        NvcVec3 bbMin = vertexData[0];
        NvcVec3 bbMax = vertexData[0];
        for (uint32_t i = 1; i < verticesCount; ++i) {
            bbMin.x = std::min(bbMin.x, vertexData[i].x);
            bbMin.y = std::min(bbMin.y, vertexData[i].y);
            bbMin.z = std::min(bbMin.z, vertexData[i].z);
            bbMax.x = std::max(bbMax.x, vertexData[i].x);
            bbMax.y = std::max(bbMax.y, vertexData[i].y);
            bbMax.z = std::max(bbMax.z, vertexData[i].z);
        }

        // Выделяем CollisionHull и заполняем его данные
        CollisionHull* hull = new CollisionHull;
        hull->pointsCount = 8;
        hull->indicesCount = 24;       // 6 граней по 4 вершины
        hull->polygonDataCount = 6;      // 6 граней бокса

        hull->points = new NvcVec3[hull->pointsCount];
        hull->indices = new uint32_t[hull->indicesCount];
        hull->polygonData = new HullPolygon[hull->polygonDataCount];

        // Определяем 8 вершин bounding box-а:
        // v0 = (min.x, min.y, min.z)
        // v1 = (max.x, min.y, min.z)
        // v2 = (max.x, max.y, min.z)
        // v3 = (min.x, max.y, min.z)
        // v4 = (min.x, min.y, max.z)
        // v5 = (max.x, min.y, max.z)
        // v6 = (max.x, max.y, max.z)
        // v7 = (min.x, max.y, max.z)
        hull->points[0] = bbMin;
        hull->points[1] = { bbMax.x, bbMin.y, bbMin.z };
        hull->points[2] = { bbMax.x, bbMax.y, bbMin.z };
        hull->points[3] = { bbMin.x, bbMax.y, bbMin.z };
        hull->points[4] = { bbMin.x, bbMin.y, bbMax.z };
        hull->points[5] = { bbMax.x, bbMin.y, bbMax.z };
        hull->points[6] = { bbMax.x, bbMax.y, bbMax.z };
        hull->points[7] = { bbMin.x, bbMax.y, bbMax.z };

        // Определяем индексы для граней (каждая грань представлена 4-мя индексами):
        // Грань 0 (левая, x = bbMin.x): v0, v4, v7, v3
        hull->indices[0] = 0;
        hull->indices[1] = 4;
        hull->indices[2] = 7;
        hull->indices[3] = 3;
        // Грань 1 (правая, x = bbMax.x): v1, v2, v6, v5
        hull->indices[4] = 1;
        hull->indices[5] = 2;
        hull->indices[6] = 6;
        hull->indices[7] = 5;
        // Грань 2 (нижняя, y = bbMin.y): v0, v1, v5, v4
        hull->indices[8]  = 0;
        hull->indices[9]  = 1;
        hull->indices[10] = 5;
        hull->indices[11] = 4;
        // Грань 3 (верхняя, y = bbMax.y): v3, v7, v6, v2
        hull->indices[12] = 3;
        hull->indices[13] = 7;
        hull->indices[14] = 6;
        hull->indices[15] = 2;
        // Грань 4 (задняя, z = bbMin.z): v0, v3, v2, v1
        hull->indices[16] = 0;
        hull->indices[17] = 3;
        hull->indices[18] = 2;
        hull->indices[19] = 1;
        // Грань 5 (передняя, z = bbMax.z): v4, v5, v6, v7
        hull->indices[20] = 4;
        hull->indices[21] = 5;
        hull->indices[22] = 6;
        hull->indices[23] = 7;

        // Заполняем данные для каждого полигона (граня):
        // Для каждого вычисляем уравнение плоскости ax+by+cz+d = 0.
        // Так как грани осесимметричны, нормали совпадают с осями.
        // Грань 0: левая (x = bbMin.x) -> нормаль (-1, 0, 0), d = bbMin.x.
        hull->polygonData[0].vertexCount = 4;
        hull->polygonData[0].indexBase   = 0;
        hull->polygonData[0].plane[0]      = -1.0f;
        hull->polygonData[0].plane[1]      =  0.0f;
        hull->polygonData[0].plane[2]      =  0.0f;
        hull->polygonData[0].plane[3]      =  bbMin.x;
        // Грань 1: правая (x = bbMax.x) -> нормаль (1, 0, 0), d = -bbMax.x.
        hull->polygonData[1].vertexCount = 4;
        hull->polygonData[1].indexBase   = 4;
        hull->polygonData[1].plane[0]      =  1.0f;
        hull->polygonData[1].plane[1]      =  0.0f;
        hull->polygonData[1].plane[2]      =  0.0f;
        hull->polygonData[1].plane[3]      = -bbMax.x;
        // Грань 2: нижняя (y = bbMin.y) -> нормаль (0, -1, 0), d = bbMin.y.
        hull->polygonData[2].vertexCount = 4;
        hull->polygonData[2].indexBase   = 8;
        hull->polygonData[2].plane[0]      =  0.0f;
        hull->polygonData[2].plane[1]      = -1.0f;
        hull->polygonData[2].plane[2]      =  0.0f;
        hull->polygonData[2].plane[3]      =  bbMin.y;
        // Грань 3: верхняя (y = bbMax.y) -> нормаль (0, 1, 0), d = -bbMax.y.
        hull->polygonData[3].vertexCount = 4;
        hull->polygonData[3].indexBase   = 12;
        hull->polygonData[3].plane[0]      =  0.0f;
        hull->polygonData[3].plane[1]      =  1.0f;
        hull->polygonData[3].plane[2]      =  0.0f;
        hull->polygonData[3].plane[3]      = -bbMax.y;
        // Грань 4: задняя (z = bbMin.z) -> нормаль (0, 0, -1), d = bbMin.z.
        hull->polygonData[4].vertexCount = 4;
        hull->polygonData[4].indexBase   = 16;
        hull->polygonData[4].plane[0]      =  0.0f;
        hull->polygonData[4].plane[1]      =  0.0f;
        hull->polygonData[4].plane[2]      = -1.0f;
        hull->polygonData[4].plane[3]      =  bbMin.z;
        // Грань 5: передняя (z = bbMax.z) -> нормаль (0, 0, 1), d = -bbMax.z.
        hull->polygonData[5].vertexCount = 4;
        hull->polygonData[5].indexBase   = 20;
        hull->polygonData[5].plane[0]      =  0.0f;
        hull->polygonData[5].plane[1]      =  0.0f;
        hull->polygonData[5].plane[2]      =  1.0f;
        hull->polygonData[5].plane[3]      = -bbMax.z;

        return hull;
    }

    // Освобождение памяти, выделенной для CollisionHull
    virtual void releaseCollisionHull(CollisionHull* hull) const override 
    {
        if (hull) 
        {
            delete[] hull->points;
            delete[] hull->indices;
            delete[] hull->polygonData;
            delete hull;
        }
    }
};

} // namespace Blast
} // namespace Nv