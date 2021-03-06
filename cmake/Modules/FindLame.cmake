
FIND_PATH(LAME_INCLUDE_DIR lame/lame.h /usr/include /usr/local/include)
FIND_LIBRARY(LAME_LIBRARY NAMES mp3lame PATH /usr/lib /usr/local/lib)

IF (LAME_INCLUDE_DIR AND LAME_LIBRARY)
        SET(LAME_FOUND TRUE)
ENDIF (LAME_INCLUDE_DIR AND LAME_LIBRARY)

IF (LAME_FOUND)
        IF (NOT lame_FIND_QUIETLY)
                MESSAGE(STATUS "Found lame: ${LAME_INCLUDE_DIR}/lame/lame.h ${LAME_LIBRARY}")
        ENDIF (NOT lame_FIND_QUIETLY)
ELSE (LAME_FOUND)
        IF (lame_FIND_REQUIRED)
                MESSAGE(FATAL_ERROR "Could not find lame")
        ENDIF (lame_FIND_REQUIRED)
ENDIF (LAME_FOUND)

