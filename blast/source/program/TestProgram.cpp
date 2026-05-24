#include <iostream>
#include <NvBlastExtUnity.h>
#include <NvBlastExtUnityConfigs.h>
#include <NvBlastFracturer.h>

Mesh* createMesh();
Fracturer* createFracturer();
void log(int type, const char* msg, const char* file, int line);

int main()
{
    log(0, "Creating mesh...", __FILE__, __LINE__);
    Mesh* mesh = createMesh();

    log(0, "Creating fracturer...", __FILE__, __LINE__);
    Fracturer* fracturer = createFracturer();

    log(0, "Fracturing...", __FILE__, __LINE__);
    ConvexMeshBuilder* collisionBuilder = NvBlastExtUnityCreateCollisionBuilder();
    AuthoringResult* result = NvBlastExtUnityFractureMesh(mesh, 1, fracturer, collisionBuilder, log);

    log(0, "Fracturing completed", __FILE__, __LINE__);

    int resultChunksCOunt = result->chunkCount;
    printf("Result chunks count: %d\n", resultChunksCOunt);

    return 0;
}

Fracturer* createFracturer()
{
    // VoronoiConfiguration voronoiConfig(5);
    // return NvBlastExtUnityCreateVoronoiFracturer(voronoiConfig);

    uint32_t width = 5;
    uint32_t height = 5;
    uint32_t bytesCount = width * height * 3;
    uint8_t* bitmap = new uint8_t[bytesCount];
    for (uint32_t i = 0; i < width; i++)
    for (uint32_t j = 0; j < height; j++)
    {
        if (i == 2 || j == 2)
        {
            bitmap[(i * width + j) * 3] = 255;
            bitmap[(i * width + j) * 3 + 1] = 255;
            bitmap[(i * width + j) * 3 + 2] = 255;
        }
        else
        {
            bitmap[(i * width + j) * 3] = 0;
            bitmap[(i * width + j) * 3 + 1] = 0;
            bitmap[(i * width + j) * 3 + 2] = 0;
        }
    }

    CutOutConfiguration cutOutConfig({0, 0, 0}, {1, 0, 0}, bitmap, width, height);
    return NvBlastExtUnityCreateCutOutFracturer(cutOutConfig);
}

Mesh* createMesh()
{
    uint32_t verticesCount = 24;
    uint32_t indicesCount = 36;

    // Simple cube with unit size
    const NvcVec3* position = new NvcVec3[verticesCount]{
        {0.50, -0.50, 0.50},    {-0.50, -0.50, 0.50},   {0.50, 0.50, 0.50},     {-0.50, 0.50, 0.50}, 
        {0.50, 0.50, -0.50},    {-0.50, 0.50, -0.50},   {0.50, -0.50, -0.50},   {-0.50, -0.50, -0.50}, 
        {0.50, 0.50, 0.50},     {-0.50, 0.50, 0.50},    {0.50, 0.50, -0.50},    {-0.50, 0.50, -0.50}, 
        {0.50, -0.50, -0.50},   {0.50, -0.50, 0.50},    {-0.50, -0.50, 0.50},   {-0.50, -0.50, -0.50}, 
        {-0.50, -0.50, 0.50},   {-0.50, 0.50, 0.50},    {-0.50, 0.50, -0.50},   {-0.50, -0.50, -0.50}, 
        {0.50, -0.50, -0.50},   {0.50, 0.50, -0.50},    {0.50, 0.50, 0.50},     {0.50, -0.50, 0.50}};

    const NvcVec3* normals = new NvcVec3[verticesCount]{
        {0.00, 0.00, 1.00},     {0.00, 0.00, 1.00},     {0.00, 0.00, 1.00},     {0.00, 0.00, 1.00},     {0.00, 1.00, 0.00},     {0.00, 1.00, 0.00}, 
        {0.00, 0.00, -1.00},    {0.00, 0.00, -1.00},    {0.00, 1.00, 0.00},     {0.00, 1.00, 0.00},     {0.00, 0.00, -1.00},    {0.00, 0.00, -1.00}, 
        {0.00, -1.00, 0.00},    {0.00, -1.00, 0.00},    {0.00, -1.00, 0.00},    {0.00, -1.00, 0.00},    {-1.00, 0.00, 0.00},    {-1.00, 0.00, 0.00}, 
        {-1.00, 0.00, 0.00},    {-1.00, 0.00, 0.00},    {1.00, 0.00, 0.00},     {1.00, 0.00, 0.00},     {1.00, 0.00, 0.00},     {1.00, 0.00, 0.00}};

    const NvcVec2* uv = new NvcVec2[verticesCount]{
        {0.00, 0.00}, {1.00, 0.00}, {0.00, 1.00}, {1.00, 1.00}, {0.00, 1.00}, {1.00, 1.00}, 
        {0.00, 1.00}, {1.00, 1.00}, {0.00, 0.00}, {1.00, 0.00}, {0.00, 0.00}, {1.00, 0.00}, 
        {0.00, 0.00}, {0.00, 1.00}, {1.00, 1.00}, {1.00, 0.00}, {0.00, 0.00}, {0.00, 1.00}, 
        {1.00, 1.00}, {1.00, 0.00}, {0.00, 0.00}, {0.00, 1.00}, {1.00, 1.00}, {1.00, 0.00}
    };

    // 36 indices for 12 triangles (6 faces, 2 triangles per face, 3 indices per triangle)
    const uint32_t* triangleIndices = new uint32_t[indicesCount]{
        0, 2, 3, 0, 3, 1, 8, 4, 5, 8, 5, 9, 10, 6, 7, 10, 7, 11, 12, 13, 
        14, 12, 14, 15, 16, 17, 18, 16, 18, 19, 20, 21, 22, 20, 22, 23
    };

    Mesh* mesh = NvBlastExtUnityCreateMesh(position, normals, uv, verticesCount, triangleIndices, indicesCount);
    return mesh;
}

void log(int type, const char* msg, const char* file, int line)
{
    std::cout << "Log: " << msg << " (" << file << ":" << line << ")" << std::endl;
}