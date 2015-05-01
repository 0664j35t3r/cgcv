project(cg1framework)

file(GLOB HEADERS "*.h")
file(GLOB SOURCES "*.cpp")

include_directories(${CMAKE_SOURCE_DIR}/cgcvcommon ${CMAKE_SOURCE_DIR}/cg/common ${CMAKE_SOURCE_DIR}/cg/common/cg1framework)
add_library(cg1framework STATIC ${HEADERS} ${SOURCES})
