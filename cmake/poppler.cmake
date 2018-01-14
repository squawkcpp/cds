# ---------------------             poppler               ---------------------
ExternalProject_Add(
  poppler
  URL "https://poppler.freedesktop.org/poppler-0.62.0.tar.xz"
  DOWNLOAD_NO_PROGRESS 1
  #CONFIGURE_COMMAND ""
  #BUILD_COMMAND cd <SOURCE_DIR> && make &> ev_build.log
  INSTALL_COMMAND ""
  UPDATE_COMMAND ""
  PATCH_COMMAND ""
  BUILD_BYPRODUCTS "opencv2-prefix/src/opencv2-build/poppler.a"
)
ExternalProject_Get_Property(poppler binary_dir)
ExternalProject_Get_Property(poppler source_dir)
set(POPPLER_LIBRARY ${binary_dir}/${CMAKE_FIND_LIBRARY_PREFIXES}poppler.a )
set(POPPLER_INCLUDE_DIR ${source_dir}/../)

set(POPPLER_LIBRARY_NAME pplr)
add_library(${POPPLER_LIBRARY_NAME} STATIC IMPORTED)
set_property(TARGET ${POPPLER_LIBRARY_NAME} PROPERTY IMPORTED_LOCATION ${POPPLER_LIBRARY} )
add_dependencies(${POPPLER_LIBRARY_NAME} poppler)
