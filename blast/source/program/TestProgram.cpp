// Scratch harness for driving the bridge without launching an editor: it builds a hardcoded
// cube, fractures it through the session C-API and reports the chunk count. Edit fracture() to
// select the operation under test.

#include <iostream>
#include <NvBlastExtBridge.h>
#include <NvBlastExtBridgeSession.h>
#include <NvBlastExtBridgeConfigs.h>

Mesh* createMesh();
int32_t fracture(NvBlastExtBridgeFractureSession* session, int32_t chunkId);
void log(int type, const char* msg, const char* file, int line);

int main()
{
    log(0, "Creating mesh...", __FILE__, __LINE__);
    Mesh* mesh = createMesh();

    log(0, "Creating session...", __FILE__, __LINE__);
    NvBlastExtBridgeFractureSession* session = NvBlastExtBridgeSessionCreate(log);

    const int32_t rootId = 0;
    if (NvBlastExtBridgeSessionSetSourceMeshes(session, &mesh, 1, &rootId) != NvBlastExtBridgeSessionResult_Success)
    {
        log(0, "Failed to set the source mesh", __FILE__, __LINE__);
        return 1;
    }

    log(0, "Fracturing...", __FILE__, __LINE__);
    if (fracture(session, rootId) != NvBlastExtBridgeSessionResult_Success)
    {
        log(0, "Fracture failed", __FILE__, __LINE__);
        return 1;
    }

    log(0, "Finalizing...", __FILE__, __LINE__);
    ConvexMeshBuilder* collisionBuilder = NvBlastExtBridgeCreateCollisionBuilder();
    AuthoringResult*   result           = NvBlastExtBridgeSessionFinalize(session, collisionBuilder, 1, -1);

    if (result == nullptr)
    {
        log(0, "Finalize failed", __FILE__, __LINE__);
        return 1;
    }

    printf("Result chunks count: %d\n", result->chunkCount);

    NvBlastExtBridgeReleaseAuthoringResult(*collisionBuilder, result);
    NvBlastExtBridgeReleaseCollisionBuilder(collisionBuilder);
    NvBlastExtBridgeSessionRelease(session);
    NvBlastExtBridgeReleaseMesh(mesh);

    return 0;
}

// Swap the body for whichever operation is being debugged.
int32_t fracture(NvBlastExtBridgeFractureSession* session, int32_t chunkId)
{
    // VoronoiConfiguration voronoiConfig(5);
    // return NvBlastExtBridgeSessionFractureVoronoi(session, chunkId, voronoiConfig, 0);

    const uint32_t width      = 5;
    const uint32_t height     = 5;
    const uint32_t bytesCount = width * height * 3;

    uint8_t* bitmap = new uint8_t[bytesCount];
    for (uint32_t i = 0; i < width; i++)
    for (uint32_t j = 0; j < height; j++)
    {
        const uint8_t value = (i == 2 || j == 2) ? 255 : 0;
        bitmap[(i * width + j) * 3]     = value;
        bitmap[(i * width + j) * 3 + 1] = value;
        bitmap[(i * width + j) * 3 + 2] = value;
    }

    NvBlastExtBridgeCutoutConfiguration config;
    config.point                      = { 0, 0, 0 };
    config.normal                     = { 1, 0, 0 };
    config.bitmap                     = bitmap;
    config.width                      = width;
    config.height                     = height;
    config.scale                      = { -1.0f, -1.0f };  // Fit the pattern to the chunk bounds
    config.aperture                   = 0.0f;
    config.isRelativeTransform        = 1;
    config.useSmoothing               = 0;
    config.segmentationErrorThreshold = 0.001f;
    config.snapThreshold              = 1.0f;
    config.periodic                   = 0;
    config.expandGaps                 = 1;
    config.noise                      = NoiseConfiguration();

    const int32_t result = NvBlastExtBridgeSessionFractureCutout(session, chunkId, &config, 0);
    delete[] bitmap;
    return result;
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

    Mesh* mesh = NvBlastExtBridgeCreateMesh(position, normals, uv, verticesCount, triangleIndices, indicesCount);
    return mesh;
}

void log(int type, const char* msg, const char* file, int line)
{
    std::cout << "Log: " << msg << " (" << file << ":" << line << ")" << std::endl;
}