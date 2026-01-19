add_library(CobaltIO STATIC)
target_compile_features(CobaltIO PUBLIC cxx_std_20)

set(COBALT_IO_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/mitsuba_reader.h
    ${CMAKE_CURRENT_SOURCE_DIR}/obj_reader.h
    ${CMAKE_CURRENT_SOURCE_DIR}/exr_reader.h
    ${CMAKE_CURRENT_SOURCE_DIR}/png_reader.h
)

set(COBALT_IO_PRIVATE_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/private/xml_utilities.h)

set(COBALT_IO_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/mitsuba_reader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/obj_reader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/exr_reader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/png_reader.cpp
)

find_package(LibXml2 REQUIRED)
find_package(OpenEXR REQUIRED)
find_package(PNG REQUIRED)

target_sources(CobaltIO PUBLIC ${COBALT_IO_HEADERS} PRIVATE ${COBALT_IO_PRIVATE_HEADERS} ${COBALT_IO_SOURCES})
target_include_directories(CobaltIO PUBLIC ${COBALT_ROOT_DIR} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_include_directories(CobaltIO PRIVATE ${OpenEXR_includedir})
target_link_libraries(CobaltIO PUBLIC CobaltCore CobaltMath PRIVATE OpenEXR::OpenEXR LibXml2::LibXml2 png rgb2spec)