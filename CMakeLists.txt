cmake_minimum_required(VERSION 3.5.0)
project(BlastCustom VERSION 5.0.6 LANGUAGES C CXX)
set (CMAKE_CXX_STANDARD 14)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

add_subdirectory(${PROJECT_SOURCE_DIR}/blast)
