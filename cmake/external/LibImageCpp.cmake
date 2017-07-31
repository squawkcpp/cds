include(ExternalProject)
# - download and build LibImageCpp
# Once done this will define
#  LIB_IMAGE_INCLUDE_DIR - The LibImageCpp include directory
#  LIB_IMAGE_LIBRARIES - The libraries needed to use LibImageCpp

ExternalProject_Add(
  libimagecpp_master
  URL "https://github.com/cppwrappers/LibImageCpp/archive/master.zip"
  INSTALL_COMMAND ""
  UPDATE_COMMAND ""
  PATCH_COMMAND ""
  BUILD_BYPRODUCTS libimagecpp_master-prefix/src/libimagecpp_master-build/libimagecpp/libimagecpp.a
)
ExternalProject_Get_Property(libimagecpp_master source_dir)
set(LIB_IMAGE_INCLUDE_DIR ${source_dir}/libimagecpp/)
ExternalProject_Get_Property(libimagecpp_master binary_dir)
set(LIB_IMAGE_LIBRARIES ${binary_dir}/libimagecpp/${CMAKE_FIND_LIBRARY_PREFIXES}imagecpp.a)
set(LIBIMAGE_LIBRARY LibImageCpp)
add_library(${LIBIMAGE_LIBRARY} UNKNOWN IMPORTED)
set_property(TARGET ${LIBIMAGE_LIBRARY} PROPERTY IMPORTED_LOCATION ${LIB_IMAGE_LIBRARIES} )
add_dependencies(${LIBIMAGE_LIBRARY} libimagecpp_master)
