# -------------------------------------------------------------------------
# Build squawk content directory server
# -------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.3)

project( cds )

SET(MAJOR_VERSION "0")
SET(MINOR_VERSION "1")
SET(PATCH_VERSION "6" CACHE PATCH_VERSION "The package build number." )

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

add_definitions(-DASIO_STANDALONE)
add_definitions(-DCDS_VERSION="${MAJOR_VERSION}.${MINOR_VERSION}.${PATCH_VERSION}")

if(CMAKE_BUILD_TYPE MATCHES Debug)
    add_definitions(-DSPDLOG_TRACE_ON)
    add_definitions(-DSPDLOG_DEBUG_ON)
    add_definitions(-DDEBUG)
endif(CMAKE_BUILD_TYPE MATCHES Debug)

#add headers for editor
FILE(GLOB_RECURSE HeaderFiles "${PROJECT_SOURCE_DIR}/src/*.h")
add_custom_target(cds_headers SOURCES ${HeaderFiles})

# -------------------------------------------------------------------------
# ---------------------   External Dependencies       ---------------------
# -------------------------------------------------------------------------
include(ExternalProject)
include(cmake/external/LibAvCpp.cmake)
include(cmake/external/LibImageCpp.cmake)
include(cmake/external/re2.cmake)
include(cmake/external/gtest.cmake)
include(cmake/external/asio.cmake)
include(cmake/external/fmt.cmake)
include(cmake/external/lightning.cmake)
include(cmake/external/rapidjson.cmake)
include(cmake/external/rapidxml_ns.cmake)
include(cmake/external/cxxopts.cmake)
include(cmake/external/redox.cmake)
include(cmake/external/spdlog.cmake)
include(cmake/external/easyexif.cmake)

SET( INCLUDES ${INCLUDES} ${GTEST_INCLUDE_DIR} ${FMT_INCLUDE_DIR}
            ${RE2_INCLUDE_DIR} ${ASIO_INCLUDE_DIR} ${LIGHTNING_INCLUDE_DIR}
            ${RAPIDJSON_INCLUDE_DIR} ${RAPIDXML_INCLUDE_DIR} ${LIBAV_INCLUDE_DIR}
            ${LIB_IMAGE_INCLUDE_DIR} ${CXXOPTS_INCLUDE_DIR} ${REDOX_INCLUDE_DIR}
            ${SPDLOG_INCLUDE_DIR} ${EXIF_INCLUDE_DIR})

# -------------------------------------------------------------------------
# -----------------------   Find Dependencies       -----------------------
# -------------------------------------------------------------------------
find_package(Boost 1.54.0 COMPONENTS filesystem system)
find_package(CURL REQUIRED)
find_package(PCRE++ REQUIRED)
find_package(Imlib2 REQUIRED)
find_package(LibAVCodec COMPONENTS avformat avcodec avfilter avresample avutil )
find_package(Poppler REQUIRED cpp)
find_package(OpenSSL REQUIRED)
find_package(Flac++ REQUIRED)
find_package(LibArchive REQUIRED)
find_package(OpenCV REQUIRED )
find_package(Hiredis)
find_package(Libev)
find_package(LibMagic)
# find_package(Musicbrainz5)

SET( LIBS ${LIBS}
    ${Boost_LIBRARIES} ${CURL_LIBRARIES} ${IMLIB2_LIBRARIES}
    ${AVCODEC_LIBRARIES} ${POPPLER_LIBRARIES} ${OPENSSL_LIBRARIES}
    ${FLAC++_LIBRARIES} ${LIBSIGCXX2_LIBRARIES} ${LibArchive_LIBRARIES}
    ${CPP_REDIS_LIBRARIES} ${LIBHIREDIS_LIBRARIES} ${LIBEV_LIBRARIES}
    ${LibMagic_LIBRARY} ${OpenCV_LIBS} #${MUSICBRAINZ5_LIBRARIES}
    -lpcrecpp -lpthread -luuid -lswresample -lm -lz
)
if(build_tests)
    SET( LIBS ${LIBS} gtest)
endif()

SET( INCLUDES ${INCLUDES}
    ${Boost_INCLUDE_DIRS} ${LIBXML2_INCLUDE_DIR} ${CURL_INCLUDE_DIRS}
    ${SQLITE3_INCLUDE_DIR} ${IMLIB2_INCLUDE_DIR} ${AVCODEC_INCLUDES}
    ${OPENSSL_INCLUDES} ${FLAC++_INCLUDE_DIR} ${PCREPP_INCLUDE_DIR}
    ${LIBSIGCXX2_INCLUDE_DIRS} ${LibArchive_INCLUDE_DIRS}
    ${LibMagic_INCLUDE_DIR} #${MUSICBRAINZ5_INCLUDE_DIRS}
)

# -------------------------------------------------------------------------
# -----------------------   Compile Binary          -----------------------
# -------------------------------------------------------------------------
aux_source_directory( ${PROJECT_SOURCE_DIR}/src CDS_SOURCES )
aux_source_directory( src/modules/ CDS_SOURCES_MODULES )
aux_source_directory( src/utils/ CDS_SOURCES_UTILS )
include_directories( ${INCLUDES} ${GTEST_INCLUDE_DIR})
add_executable(cds ${PROJECT_SOURCE_DIR}/main.cpp ${CDS_SOURCES} ${CDS_SOURCES_MODULES} ${CDS_SOURCES_UTILS})
target_link_libraries(cds re2 lightning redox fmt spdlog LibAvCpp LibImageCpp exif asio ${LIBS})
install(TARGETS cds RUNTIME DESTINATION bin)

# -------------------------------------------------------------------------
# -----------------------   Manage Test Cases       -----------------------
# -------------------------------------------------------------------------
if (build_tests)
    aux_source_directory( ${PROJECT_SOURCE_DIR}/test CDS_SOURCES_TEST )
    enable_testing()
    include_directories(${ROOT} ${INCLUDES} ${GTEST_INCLUDE_DIR})
    add_executable(test_cds ${CDS_SOURCES_TEST} ${CDS_SOURCES} ${CDS_SOURCES_MODULES} ${CDS_SOURCES_UTILS})
    target_link_libraries(test_cds ${GTEST_MAIN_PATH} cxxopts re2 lightning rapidjson rapidxml
        LibAvCpp LibImageCpp redox fmt exif spdlog asio gtest ${LIBS} )
    add_test(cds_tests test_cds)
endif()

# -------------------------------------------------------------------------
# -----------------------   Doxygen                 -----------------------
# -------------------------------------------------------------------------
find_package(Doxygen)
if(DOXYGEN_FOUND)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
add_custom_target(doc ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating API documentation with Doxygen" VERBATIM
)
endif(DOXYGEN_FOUND)

# -------------------------------------------------------------------------
# -----------------------   Generate Package        -----------------------
# -------------------------------------------------------------------------
IF(EXISTS "${CMAKE_ROOT}/Modules/CPack.cmake")
INCLUDE(InstallRequiredSystemLibraries)

SET(CPACK_SET_DESTDIR "on")
SET(CPACK_PACKAGING_INSTALL_PREFIX "/tmp")
SET(CPACK_GENERATOR "DEB")

set(CPACK_ARCHIVE_COMPONENT_INSTALL ON)

SET(CPACK_PACKAGE_DESCRIPTION "loudlabs media server")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "squawk content directory server.")
SET(CPACK_PACKAGE_VENDOR "loudlabs")
SET(CPACK_PACKAGE_CONTACT "squawkcpp@gmail.com")
SET(CPACK_PACKAGE_VERSION_MAJOR "${MAJOR_VERSION}")
SET(CPACK_PACKAGE_VERSION_MINOR "${MINOR_VERSION}")
SET(CPACK_PACKAGE_VERSION_PATCH "${PATCH_VERSION}")
SET(CPACK_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}_${MAJOR_VERSION}.${MINOR_VERSION}.${CPACK_PACKAGE_VERSION_PATCH}")
SET(CPACK_SOURCE_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}_${MAJOR_VERSION}.${MINOR_VERSION}.${CPACK_PACKAGE_VERSION_PATCH}")

SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libpcrecpp0v5, libimlib2, libavcodec-ffmpeg56, libavformat-ffmpeg56, libavutil-ffmpeg54, libpoppler-cpp0, libboost-filesystem1.58.0")
SET(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
SET(CPACK_DEBIAN_PACKAGE_SECTION "media")
SET(CPACK_DEBIAN_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR})
SET(CPACK_COMPONENTS_ALL applications Libraries ApplicationData)
INCLUDE(CPack)

ENDIF(EXISTS "${CMAKE_ROOT}/Modules/CPack.cmake")
