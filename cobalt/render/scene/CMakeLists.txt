find_package(LibXml2 REQUIRED)

set(COBALT_RENDER_SCENE_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/scene.h
    ${CMAKE_CURRENT_SOURCE_DIR}/scene_factory.h)

set(COBALT_RENDER_PRIVATE_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/private/mitsuba_utilities.h
    ${CMAKE_CURRENT_SOURCE_DIR}/private/obj_utilities.h)

set(COBALT_RENDER_SCENE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/scene.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/scene_factory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/private/mitsuba_utilities.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/private/obj_utilities.cpp)

target_sources(CobaltRender PUBLIC ${COBALT_RENDER_SCENE_HEADERS} PRIVATE ${COBALT_SCENE_PRIVATE_HEADERS} ${COBALT_RENDER_SCENE_SOURCES})
target_include_directories(CobaltRender AFTER PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${COBALT_SCENE_PRIVATE_HEADERS})
target_link_libraries(CobaltRender PRIVATE LibXml2::LibXml2)