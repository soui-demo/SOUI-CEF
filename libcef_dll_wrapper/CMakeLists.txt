#
# 1394020320@qq.com
#

add_definitions(-DNOMINMAX -DWIN32_LEAN_AND_MEAN -D_HAS_EXCEPTIONS=0 -DWRAPPING_CEF_SHARED)

include_directories(${CMAKE_SOURCE_DIR}/cef_3029)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB_RECURSE CURRENT_HEADERS  *.h *.hpp)
file(GLOB_RECURSE CURRENT_SRCS  *.cpp *.cc)

source_group("Header Files" FILES ${CURRENT_HEADERS})
source_group("Source Files" FILES ${CURRENT_SRCS})

add_library(libcef_dll_wrapper STATIC ${CURRENT_HEADERS} ${CURRENT_SRCS})

set_target_properties(libcef_dll_wrapper PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})

cotire(libcef_dll_wrapper)
