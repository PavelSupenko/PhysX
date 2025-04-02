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

# Enable AVX and FMA based on the compiler and platform
if (MSVC)
    # For Visual Studio on Windows, enable AVX2 (which includes FMA support)
    target_compile_options(NvBlastExtStress PRIVATE /arch:AVX2)
# elseif(APPLE)
    # For Clang on macOS, enable AVX and FMA
    # target_compile_options(NvBlastExtStress PRIVATE -mavx -mfma)
elseif(CMAKE_COMPILER_IS_GNUCXX)
    # For GCC on other Unix-like systems, enable AVX and FMA
    target_compile_options(NvBlastExtStress PRIVATE -mavx -mfma)
endif()


target_compile_definitions(NvBlastExtStress PRIVATE 
    LOG_COMPONENT="NvBlastExtStress"
)
