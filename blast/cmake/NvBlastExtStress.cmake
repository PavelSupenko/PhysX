set(NV_BLAST_EXT_STRESS_SOURCE
    ${COMMON_SOURCES}
    ${PROJECT_SOURCE_DIR}/source/sdk/extensions/stress/NvBlastExtStressSolver.cpp
    ${PROJECT_SOURCE_DIR}/source/shared/stress_solver/stress.cpp
)

set(NV_BLAST_EXT_STRESS_INCLUDES
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/source/sdk/common
    ${PROJECT_SOURCE_DIR}/include/extensions/stress
    ${PROJECT_SOURCE_DIR}/source/sdk/extensions/stress
    ${PROJECT_SOURCE_DIR}/include/lowlevel
    ${PROJECT_SOURCE_DIR}/include/globals
    ${PROJECT_SOURCE_DIR}/include/shared/NvFoundation
    ${PROJECT_SOURCE_DIR}/source/shared/NsFoundation/include
    ${PROJECT_SOURCE_DIR}/source/shared/stress_solver
)

add_library(NvBlastExtStress ${LIBRARIES_TYPE}
    ${NV_BLAST_EXT_STRESS_SOURCE}
)

target_include_directories(NvBlastExtStress PRIVATE
    ${NV_BLAST_EXT_STRESS_INCLUDES}
)

target_link_libraries(NvBlastExtStress
    NvBlast
    NvBlastGlobals
)

if(NV_CONFIGURATION_TYPE STREQUAL DEBUG)
    target_compile_definitions(NvBlastExtStress PRIVATE 
        LOG_COMPONENT="NvBlastExtStress"
        _CRT_NONSTDC_NO_DEPRECATE
        BOOST_USE_WINDOWS_H=1
        _DEBUG
        CARB_DEBUG=1
    )
else()
    target_compile_definitions(NvBlastExtStress PRIVATE 
        LOG_COMPONENT="NvBlastExtStress"
        _CRT_NONSTDC_NO_DEPRECATE
        BOOST_USE_WINDOWS_H=1
        NDEBUG
        CARB_DEBUG=0
)
endif()
