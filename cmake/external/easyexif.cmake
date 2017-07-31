include(ExternalProject)
# - download and build EasyExif
# Once done this will define
#  EXIF_INCLUDE_DIR - The EasyExif include directory
#  EXIF_LIBRARIES - The libraries needed to use EasyExif

ExternalProject_Add(
  easyexif_master
    BUILD_IN_SOURCE 1
    URL "https://github.com/mayanklahiri/easyexif/archive/master.zip"
  CONFIGURE_COMMAND ""
  BUILD_COMMAND "make"
  INSTALL_COMMAND ""
  UPDATE_COMMAND ""
  PATCH_COMMAND ""
  BUILD_BYPRODUCTS "easyexif_master-prefix/src/easyexif_master/exif.o"
)
ExternalProject_Get_Property(easyexif_master source_dir)
set(EXIF_INCLUDE_DIR ${source_dir}/)
ExternalProject_Get_Property(easyexif_master binary_dir)
set(EXIF_LIBRARIES ${source_dir}/exif.o)
set(EASYEXIF_LIBRARY exif)
add_library(${EASYEXIF_LIBRARY} UNKNOWN IMPORTED)
set_property(TARGET ${EASYEXIF_LIBRARY} PROPERTY IMPORTED_LOCATION ${EXIF_LIBRARIES} )
add_dependencies(${EASYEXIF_LIBRARY} easyexif_master)
