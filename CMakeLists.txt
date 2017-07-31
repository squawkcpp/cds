# -------------------------------------------------------------------------
# Build content directory server
# -------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.0)

project( cds )

SET(MAJOR_VERSION "0")
SET(MINOR_VERSION "1")
SET(PATCH_VERSION "1" CACHE PATCH_VERSION "The package build number." )

# -------------------------------------------------------------------------
# -----------------------   Options and Parameters  -----------------------
# -------------------------------------------------------------------------
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
option(build_tests "Build all squawk unit tests." ON)

# -------------------------------------------------------------------------
# -----------------------   Config Toolchain        -----------------------
# -------------------------------------------------------------------------
if(CMAKE_COMPILER_IS_GNUCXX)
    add_definitions(-std=c++14)
endif()

#add headers for editor
FILE(GLOB_RECURSE HeaderFiles "${PROJECT_SOURCE_DIR}/*.h")
add_custom_target(cds_headers SOURCES ${HeaderFiles})

# -------------------------------------------------------------------------
# -----------------------   Compile Binary          -----------------------
# -------------------------------------------------------------------------
aux_source_directory( ${PROJECT_SOURCE_DIR} CDS_SOURCES )
aux_source_directory( modules/ CDS_SOURCES_MODULES )
aux_source_directory( utils/ CDS_SOURCES_UTILS )
include_directories(${ROOT} ${INCLUDES} ${GTEST_INCLUDE_DIR})
add_library(cds ${CDS_SOURCES} ${CDS_SOURCES_MODULES} ${CDS_SOURCES_UTILS})

# -------------------------------------------------------------------------
# -----------------------   Manage Test Cases       -----------------------
# -------------------------------------------------------------------------
if (build_tests)
    aux_source_directory( test/ CDS_SOURCES_TEST )
    enable_testing()
    include_directories(${ROOT} ${INCLUDES} ${GTEST_INCLUDE_DIR})
    add_executable(test_cds ${CDS_SOURCES_TEST} ${CDS_SOURCES} ${CDS_SOURCES_MODULES} ${CDS_SOURCES_UTILS})
    target_link_libraries(test_cds ${GTEST_MAIN_PATH} cxxopts re2 lightning rapidjson rapidxml
        LibAvCpp LibImageCpp redox fmt exif spdlog gtest ${LIBS} )
    add_test(cds-tests test_cds)
endif()
