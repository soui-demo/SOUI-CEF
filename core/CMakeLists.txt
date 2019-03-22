#
# 1394020320@qq.com
#

#add_definitions(-D_CRT_SECURE_NO_WARNINGS)

include_directories(${SOUI_ROOT}/config)
include_directories(${SOUI_ROOT}/components)
include_directories(${SOUI_ROOT}/utilities/include)
include_directories(${SOUI_ROOT}/SOUI/include)
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

add_library(core SHARED ${CURRENT_HEADERS} ${CURRENT_SRCS} ${CURRENT_SOUIS} ${CURRENT_RC})

add_dependencies(core libcef_dll_wrapper)
target_link_libraries(core
        debug souid optimized soui
        debug utilitiesd optimized utilities
        libcef
        libcef_dll_wrapper
)

set_target_properties(core PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
set_target_properties(core PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})

cotire(core)
