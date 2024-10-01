cmake_minimum_required(VERSION 3.5.0)
project(Blast VERSION 5.0.6)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

set(LIBRARIES_TYPE SHARED)
set(NV_CONFIGURATION_TYPE RELEASE) # Use DEBUG or RELEASE

## === Common === ##
set(COMMON_SOURCES
    ${PROJECT_SOURCE_DIR}/source/sdk/common/NvBlastAssert.cpp
    ${PROJECT_SOURCE_DIR}/source/sdk/common/NvBlastAtomic.cpp
    ${PROJECT_SOURCE_DIR}/source/sdk/common/NvBlastTime.cpp
    ${PROJECT_SOURCE_DIR}/source/sdk/common/NvBlastTimers.cpp
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
