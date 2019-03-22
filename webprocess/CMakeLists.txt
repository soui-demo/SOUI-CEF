#
# 1394020320@qq.com
#

include_directories(${CMAKE_SOURCE_DIR}/cef_3029)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB_RECURSE CURRENT_HEADERS  *.h *.hpp)
file(GLOB_RECURSE CURRENT_SRCS  *.cpp *.cc)
file(GLOB_RECURSE CURRENT_SOUIS  *.xml *.idx *.ico)
file(GLOB_RECURSE CURRENT_RC *.rc *.rc2)

source_group("Header Files" FILES ${CURRENT_HEADERS})
source_group("Source Files" FILES ${CURRENT_SRCS})
source_group("SoUI Resouece" FILES ${CURRENT_SOUIS})
source_group("Resource Files" FILES ${CURRENT_RC})

link_directories(${CMAKE_SOURCE_DIR}/bin)
link_directories(${CMAKE_SOURCE_DIR}/cef_3029/Release)
link_directories(${SOUI_ROOT}/bin)

add_executable(webprocess WIN32 ${CURRENT_HEADERS} ${CURRENT_SRCS} ${CURRENT_SOUIS} ${CURRENT_RC})

add_dependencies(webprocess libcef_dll_wrapper)

target_link_libraries(webprocess
        libcef
        libcef_dll_wrapper
)

set_target_properties(webprocess PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})

cotire(webprocess)
