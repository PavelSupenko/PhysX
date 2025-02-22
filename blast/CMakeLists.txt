cmake_minimum_required(VERSION 3.5.0)
project(Blast VERSION 5.0.6)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(LIBRARIES_TYPE SHARED)

message("Libraries configuration type: ${NV_CONFIGURATION_TYPE}")
message("CMake Android ARM NEON: ${CMAKE_ANDROID_ARM_NEON}")
message("Android ARM NEON: ${ANDROID_ARM_NEON}")

## === Common sources === ##
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
include(cmake/NvBlastExtPhysX.cmake)

## === UnitTests === ##
include(cmake/UnitTests.cmake)

## === Compile flags === ##

string(TOLOWER ${NV_CONFIGURATION_TYPE} NV_CONFIGURATION_TYPE)

if(NV_CONFIGURATION_TYPE STREQUAL debug)
    add_compile_definitions(
        _CRT_NONSTDC_NO_DEPRECATE
        BOOST_USE_WINDOWS_H=1
        _DEBUG
        CARB_DEBUG=1
    )
else()
    add_compile_definitions( 
        _CRT_NONSTDC_NO_DEPRECATE
        BOOST_USE_WINDOWS_H=1
        NDEBUG
        CARB_DEBUG=0
)
endif()