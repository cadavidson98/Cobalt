set(COBALT_RENDER_DATA_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/byte_texture.h
    ${CMAKE_CURRENT_SOURCE_DIR}/camera.h
    ${CMAKE_CURRENT_SOURCE_DIR}/image_reader.h
    ${CMAKE_CURRENT_SOURCE_DIR}/render_target.h
    ${CMAKE_CURRENT_SOURCE_DIR}/resolver.h
    ${CMAKE_CURRENT_SOURCE_DIR}/texture.h)

set(COBALT_RENDER_DATA_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/byte_texture.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/camera.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/color.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/image_reader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/render_target.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/resolver.cpp)

target_sources(CobaltRender PUBLIC ${COBALT_RENDER_DATA_HEADERS} PRIVATE ${COBALT_RENDER_DATA_SOURCES})
target_include_directories(CobaltRender AFTER PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})