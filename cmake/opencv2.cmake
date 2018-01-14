# ---------------------             opencv2               ---------------------
ExternalProject_Add(
  opencv2
  URL "https://github.com/opencv/opencv/archive/3.4.0.zip"
  DOWNLOAD_NO_PROGRESS 1
  #CONFIGURE_COMMAND ""
  #BUILD_COMMAND cd <SOURCE_DIR> && make &> ev_build.log
  INSTALL_COMMAND ""
  UPDATE_COMMAND ""
  PATCH_COMMAND ""
  BUILD_BYPRODUCTS "opencv2-prefix/src/opencv2-build/libopencv.a"
)
ExternalProject_Get_Property(opencv2 binary_dir)
ExternalProject_Get_Property(opencv2 source_dir)
set(OPENCV2_LIBRARY ${binary_dir}/${CMAKE_FIND_LIBRARY_PREFIXES}opencv.a )
set(OPENCV2_INCLUDE_DIR ${source_dir}/include ${binary_dir} ${source_dir}/modules/core/include)

set(OPENCV_LIBRARY_NAME opencv)
add_library(${OPENCV_LIBRARY_NAME} STATIC IMPORTED)
set_property(TARGET ${OPENCV_LIBRARY_NAME} PROPERTY IMPORTED_LOCATION ${OPENCV2_LIBRARY} )
add_dependencies(${OPENCV_LIBRARY_NAME} hiredis)
