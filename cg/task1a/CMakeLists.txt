project(cgtask1a)

file(GLOB SOURCES ${SOURCE_WILDCARDS})

include_directories(
  ${CMAKE_SOURCE_DIR}/cgcvcommon
  ../common
  ../common/cg1framework
  ${ZLIB_INCLUDE_DIR}
  ${PNG_INCLUDE_DIR}
)

add_executable(cgtask1a
  ${SOURCES}
)
target_link_libraries(cgtask1a
  cg1framework
  cgcommon
  cgcvcommon
  tinyxml
  ${ZLIB_LIBRARIES}
  ${PNG_LIBRARIES}
)

add_submit_target(cgtask1a)