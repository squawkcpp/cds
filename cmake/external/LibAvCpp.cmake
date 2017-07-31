include(ExternalProject)
# - download and build LibAvCpp
# Once done this will define
#  LIBAV_INCLUDE_DIR - The LibAvCpp include directory
#  LIBAV_LIBRARIES - The libraries needed to use LibAvCpp

ExternalProject_Add(
  libavcpp_master
  URL "https://github.com/cppwrappers/LibAvCpp/archive/master.zip"
  INSTALL_COMMAND ""
  UPDATE_COMMAND ""
  PATCH_COMMAND ""
  BUILD_BYPRODUCTS libavcpp_master-prefix/src/libavcpp_master-build/libavcpp/libavcpp.a
)
ExternalProject_Get_Property(libavcpp_master source_dir)
set(LIBAV_INCLUDE_DIR ${source_dir}/libavcpp/)
ExternalProject_Get_Property(libavcpp_master binary_dir)
set(LIBAV_LIBRARIES ${binary_dir}/libavcpp/${CMAKE_FIND_LIBRARY_PREFIXES}avcpp.a)
set(LIBAV_LIBRARY LibAvCpp)
add_library(${LIBAV_LIBRARY} UNKNOWN IMPORTED)
set_property(TARGET ${LIBAV_LIBRARY} PROPERTY IMPORTED_LOCATION ${LIBAV_LIBRARIES} )
add_dependencies(${LIBAV_LIBRARY} libavcpp_master)
