# Gtest official code manual: https://google.github.io/googletest/quickstart-cmake.html
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

#enable_testing()

set(TESTS_SOURCE
    ${COMMON_SOURCES}
    ${PROJECT_SOURCE_DIR}/source/sdk/lowlevel/NvBlastActor.cpp
    ${PROJECT_SOURCE_DIR}/source/sdk/lowlevel/NvBlastActorSerializationBlock.cpp
    ${PROJECT_SOURCE_DIR}/source/sdk/lowlevel/NvBlastAsset.cpp
    ${PROJECT_SOURCE_DIR}/source/sdk/lowlevel/NvBlastFamily.cpp
    ${PROJECT_SOURCE_DIR}/source/sdk/lowlevel/NvBlastFamilyGraph.cpp
    ${PROJECT_SOURCE_DIR}/source/sdk/toolkit/NvBlastTkTaskManager.cpp
    ${PROJECT_SOURCE_DIR}/source/shared/utils/AssetGenerator.cpp
    ${PROJECT_SOURCE_DIR}/source/test/src/unit/APITests.cpp
    ${PROJECT_SOURCE_DIR}/source/test/src/unit/ActorTests.cpp
    ${PROJECT_SOURCE_DIR}/source/test/src/unit/AssetTests.cpp
    ${PROJECT_SOURCE_DIR}/source/test/src/unit/CoreTests.cpp
    ${PROJECT_SOURCE_DIR}/source/test/src/unit/FamilyGraphTests.cpp
    ${PROJECT_SOURCE_DIR}/source/test/src/unit/MultithreadingTests.cpp
    ${PROJECT_SOURCE_DIR}/source/test/src/unit/TkCompositeTests.cpp
    ${PROJECT_SOURCE_DIR}/source/test/src/unit/TkTests.cpp
    ${PROJECT_SOURCE_DIR}/source/test/src/utils/TestAssets.cpp
)

set(TESTS_INCLUDES
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/globals
    ${PROJECT_SOURCE_DIR}/include/lowlevel
    ${PROJECT_SOURCE_DIR}/include/toolkit
    ${PROJECT_SOURCE_DIR}/include/extensions/assetutils
    ${PROJECT_SOURCE_DIR}/include/extensions/shaders
    ${PROJECT_SOURCE_DIR}/include/extensions/serialization
    ${PROJECT_SOURCE_DIR}/source/sdk/common
    ${PROJECT_SOURCE_DIR}/source/sdk/globals
    ${PROJECT_SOURCE_DIR}/source/sdk/lowlevel
    ${PROJECT_SOURCE_DIR}/source/sdk/extensions/serialization
    ${PROJECT_SOURCE_DIR}/source/test/src
    ${PROJECT_SOURCE_DIR}/source/test/src/unit
    ${PROJECT_SOURCE_DIR}/source/test/src/utils
    ${PROJECT_SOURCE_DIR}/source/shared/filebuf/include
    ${PROJECT_SOURCE_DIR}/source/shared/utils
    ${PROJECT_SOURCE_DIR}/include/shared/NvFoundation
    ${PROJECT_SOURCE_DIR}/source/shared/NsFoundation/include
    ${PROJECT_SOURCE_DIR}/source/shared/NsFileBuffer/include
    ${PROJECT_SOURCE_DIR}/source/shared/NvTask/include
)

add_executable(UnitTests
    ${TESTS_SOURCE}
)

target_include_directories(UnitTests PRIVATE
    ${TESTS_INCLUDES}
)

target_link_libraries(UnitTests
    NvBlast
    NvBlastTk
    NvBlastGlobals
    NvBlastExtAssetUtils
    NvBlastExtShaders
    NvBlastExtSerialization
    NvBlastExtTkSerialization

    GTest::gtest_main
)

include(GoogleTest)

target_compile_definitions(UnitTests PRIVATE 
        LOG_COMPONENT="UnitTests"
)
