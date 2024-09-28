set(TESTS_SOURCE
    ${COMMON_SOURCES}
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/lowlevel/NvBlastActor.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/lowlevel/NvBlastActorSerializationBlock.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/lowlevel/NvBlastAsset.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/lowlevel/NvBlastFamily.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/lowlevel/NvBlastFamilyGraph.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/toolkit/NvBlastTkTaskManager.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/shared/utils/AssetGenerator.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/test/src/unit/APITests.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/test/src/unit/ActorTests.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/test/src/unit/AssetTests.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/test/src/unit/CoreTests.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/test/src/unit/FamilyGraphTests.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/test/src/unit/MultithreadingTests.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/test/src/unit/TkCompositeTests.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/test/src/unit/TkTests.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/test/src/utils/TestAssets.cpp
)

set(TESTS_INCLUDES
    ${PROJECT_SOURCE_DIR}/blast/include
    ${PROJECT_SOURCE_DIR}/blast/include/globals
    ${PROJECT_SOURCE_DIR}/blast/include/lowlevel
    ${PROJECT_SOURCE_DIR}/blast/include/toolkit
    ${PROJECT_SOURCE_DIR}/blast/include/extensions/assetutils
    ${PROJECT_SOURCE_DIR}/blast/include/extensions/shaders
    ${PROJECT_SOURCE_DIR}/blast/include/extensions/serialization
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/common
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/globals
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/lowlevel
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/extensions/serialization
    ${PROJECT_SOURCE_DIR}/blast/source/test/src
    ${PROJECT_SOURCE_DIR}/blast/source/test/src/unit
    ${PROJECT_SOURCE_DIR}/blast/source/test/src/utils
    ${PROJECT_SOURCE_DIR}/blast/source/shared/filebuf/include
    ${PROJECT_SOURCE_DIR}/blast/source/shared/utils
    ${PROJECT_SOURCE_DIR}/blast/include/shared/NvFoundation
    ${PROJECT_SOURCE_DIR}/blast/source/shared/NsFoundation/include
    ${PROJECT_SOURCE_DIR}/blast/source/shared/NsFileBuffer/include
    ${PROJECT_SOURCE_DIR}/blast/source/shared/NvTask/include

    ${PROJECT_SOURCE_DIR}/blast/_build/target-deps/googletest/include
    ${PROJECT_SOURCE_DIR}/blast/_build/host-deps/msvc/VC/Tools/MSVC/14.16.27023/include
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
)

# use Release for release builds of libs because it causes error LNK2038: mismatch detected for '_ITERATOR_DEBUG_LEVEL'
if (NV_CONFIGURATION_TYPE STREQUAL DEBUG)
    set(GTEST_LIBRARY_PATH ${PROJECT_SOURCE_DIR}/blast/_build/target-deps/googletest/lib/vc14win64-cmake/Debug)
else()
    set(GTEST_LIBRARY_PATH ${PROJECT_SOURCE_DIR}/blast/_build/target-deps/googletest/lib/vc14win64-cmake/Release)
endif()

target_link_directories(UnitTests BEFORE PRIVATE
    ${GTEST_LIBRARY_PATH}
)

target_link_libraries(UnitTests
    gtest_main.lib
    gtest.lib
)

if(NV_CONFIGURATION_TYPE STREQUAL DEBUG)
    target_compile_definitions(UnitTests PRIVATE 
        LOG_COMPONENT="UnitTests"
        _CRT_NONSTDC_NO_DEPRECATE
        BOOST_USE_WINDOWS_H=1
        _DEBUG
        CARB_DEBUG=1
    )
else()
    target_compile_definitions(UnitTests PRIVATE 
        LOG_COMPONENT="UnitTests"
        _CRT_NONSTDC_NO_DEPRECATE
        BOOST_USE_WINDOWS_H=1
        NDEBUG
        CARB_DEBUG=0
)
endif()
