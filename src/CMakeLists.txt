# -------------------------------------------------------------------------
# Build squawk content directory server app
# -------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.3)

project( cds_app )

aux_source_directory( ${PROJECT_SOURCE_DIR} CDS_SOURCES )
aux_source_directory( ${PROJECT_SOURCE_DIR}/modules/ CDS_SOURCES_MODULES )
aux_source_directory( ${PROJECT_SOURCE_DIR}/utils/ CDS_SOURCES_UTILS )

include_directories( ${IMPORTED_INCLUDES} ${GTEST_INCLUDE_DIR})
add_library(${PROJECT_NAME} ${CDS_SOURCES} ${CDS_SOURCES_MODULES} ${CDS_SOURCES_UTILS})
target_link_libraries(${PROJECT_NAME}
    re2 lightning spdlog fmt asio
    pplr opencv rapidjson rapidxml redox
    ${CURL_LIBRARIES} ${LIBEV_LIBRARIES}
    hredis libev boost filesystem system ssl crypto Threads::Threads -ldl
)
