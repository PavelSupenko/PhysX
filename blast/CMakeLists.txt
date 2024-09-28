set(LIBRARIES_TYPE SHARED)
set(NV_CONFIGURATION_TYPE DEBUG) # Use Debug or Release

## === Common === ##
set(COMMON_SOURCES
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/common/NvBlastAssert.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/common/NvBlastAtomic.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/common/NvBlastTime.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/common/NvBlastTimers.cpp
)

## === Libraries === ##
include(cmake/NvBlast.cmake)
include(cmake/NvBlastGlobals.cmake)
include(cmake/NvBlastExtAssetUtils.cmake)
include(cmake/NvBlastExtAuthoring.cmake)
include(cmake/NvBlastExtSerialization.cmake)
include(cmake/NvBlastExtShaders.cmake)
include(cmake/NvBlastExtStress.cmake)
include(cmake/NvBlastTk.cmake)
include(cmake/NvBlastExtTkSerialization.cmake)

## === UnitTests === ##
include(cmake/UnitTests.cmake)