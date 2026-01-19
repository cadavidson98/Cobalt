project(rgb2spec LANGUAGES C)

add_library(rgb2spec STATIC)

set(RGB2SPEC_INCLUDE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/include/rgb2spec)
set(RGB2SPEC_SOURCES_PATH ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(RGB2SPEC_TABLES_PATH ${CMAKE_CURRENT_SOURCE_DIR}/tables)

configure_file(${RGB2SPEC_INCLUDE_PATH}/rgb2spec_config.h.in ${RGB2SPEC_INCLUDE_PATH}/rgb2spec_config.h)

target_sources(rgb2spec
PUBLIC ${RGB2SPEC_INCLUDE_PATH}/rgb2spec.h
${RGB2SPEC_SOURCES_PATH}/rgb2spec.c PRIVATE ${RGB2SPEC_INCLUDE_PATH}/rgb2spec_config.h)

target_include_directories(rgb2spec PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include PRIVATE ${RGB2SPEC_INCLUDE_PATH})