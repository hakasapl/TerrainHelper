cmake_minimum_required(VERSION 3.21)

# Set project name and version
set(PLUGIN_NAME TerrainHelper)
set(PLUGIN_VERSION 0.3.8)
project(${PLUGIN_NAME} VERSION ${PLUGIN_VERSION} LANGUAGES CXX)

# Set compile defitions
add_compile_definitions(PLUGIN_NAME="${PLUGIN_NAME}")
add_compile_definitions(PLUGIN_VERSION="${PLUGIN_VERSION}")

include_directories("include")

# External Packages

# Setup commonlibsse-ng
set(BUILD_TESTS OFF)
add_subdirectory("extern/commonlibsse" commonlibsse)

# clib-util
add_subdirectory("extern/CLibUtil" CLibUtil)
include_directories("extern/CLibUtil/include")

# detours
include_directories("extern/detours")

set(headers
    "include/TerrainHelper.h"
	"include/PCH.h"
)

set(sources
    "src/TerrainHelper.cpp"
    "src/main.cpp"
)

add_library("${PLUGIN_NAME}" SHARED ${sources} ${headers})

target_compile_features(${PLUGIN_NAME} PRIVATE cxx_std_23) # <--- use C++23 standard
target_precompile_headers(${PLUGIN_NAME} PRIVATE include/PCH.h) # <--- PCH.h is required!

target_include_directories(
	"${PLUGIN_NAME}"
    PUBLIC
    "include"
)

target_link_libraries(
    "${PLUGIN_NAME}"
    PRIVATE
    CommonLibSSE::CommonLibSSE
    debug ${CMAKE_SOURCE_DIR}/extern/detours/detours/Debug/detours.lib
    optimized ${CMAKE_SOURCE_DIR}/extern/detours/detours/Release/detours.lib
)

# Auto export built file
if(DEFINED OUTPUT_FOLDER)
    set(DLL_FOLDER "${OUTPUT_FOLDER}/SKSE/Plugins")

    message(STATUS "SKSE plugin output folder: ${DLL_FOLDER}")

    add_custom_command(
        TARGET "${PROJECT_NAME}"
        POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${DLL_FOLDER}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_FILE:${PROJECT_NAME}>" "${DLL_FOLDER}/$<TARGET_FILE_NAME:${PROJECT_NAME}>"
        VERBATIM
    )

    # debug build
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_custom_command(
            TARGET "${PROJECT_NAME}"
            POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_PDB_FILE:${PROJECT_NAME}>" "${DLL_FOLDER}/$<TARGET_PDB_FILE_NAME:${PROJECT_NAME}>"
            VERBATIM
        )
    endif()
endif()
