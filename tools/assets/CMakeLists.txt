set(CobaltToolsResourcesBaseDir ${CMAKE_CURRENT_SOURCE_DIR})
set(CobaltToolsResourcesBaseDir ${CMAKE_CURRENT_SOURCE_DIR})
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/assets.h.in ${CMAKE_CURRENT_SOURCE_DIR}/assets.h)

add_library(CobaltToolsAssets INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/assets.h)
target_include_directories(CobaltToolsAssets INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})