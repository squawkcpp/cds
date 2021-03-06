FIND_LIBRARY(ID3_LIBRARY NAMES libid3 id3)
FIND_PATH(ID3_INCLUDE_DIR id3/tag.h)
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(ID3 DEFAULT_MSG ID3_LIBRARY ID3_INCLUDE_DIR)
MARK_AS_ADVANCED(ID3_LIBRARY)
MARK_AS_ADVANCED(ID3_INCLUDE_DIR)