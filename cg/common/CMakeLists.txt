project(cgcommon)

set(HEADERS
  cgmath.h
  vector.h
  matrix.h
  color.h
  surface.h
  formats.h
)

set(SOURCES
  surface.cpp
)

include_directories(${CMAKE_SOURCE_DIR}/cgcvcommon ${ZLIB_INCLUDE_DIR} ${PNG_INCLUDE_DIR})
add_library(cgcommon STATIC ${HEADERS} ${SOURCES})

add_subdirectory_if_exists(cg1framework)
if (BUILD_CG2)
  if (APPLE)
    find_path(GLFW_INCLUDE_DIR DOC "GLFW include directory"
              NAMES glfw.h PATH_SUFFIXES gl 
              PATHS C:/glfw-2.7.2/include C:/glfw/include /usr/include/ /usr/local/include/ /usr/include/GL /usr/local/include/GL/)
    find_library(GLFW_LIBRARIES DOC "GLFW libraries"
                 NAMES GLFW glfw GLFW.lib
                 PATHS /usr/local/lib /usr/lib/)
  else()
    add_subdirectory_if_exists(glfw-2.7.2)
  endif()
endif ()
