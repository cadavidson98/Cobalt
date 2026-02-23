add_library(CobaltCore STATIC)
target_compile_features(CobaltCore PUBLIC cxx_std_20)

set(COBALT_CORE_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/algorithms.h
    ${CMAKE_CURRENT_SOURCE_DIR}/callback.h
    ${CMAKE_CURRENT_SOURCE_DIR}/dynamic_array.h
    ${CMAKE_CURRENT_SOURCE_DIR}/logging.h
    ${CMAKE_CURRENT_SOURCE_DIR}/morton_encoding.h
    ${CMAKE_CURRENT_SOURCE_DIR}/size_types.h
    ${CMAKE_CURRENT_SOURCE_DIR}/string_utilities.h
    ${CMAKE_CURRENT_SOURCE_DIR}/system.h
    ${CMAKE_CURRENT_SOURCE_DIR}/vertex_buffer.h)

set(COBALT_CORE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/logging.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/system.cpp)

target_compile_definitions(CobaltCore PUBLIC COBALT_LOG_DEBUG COBALT_LOG_INFO COBALT_LOG_WARN COBALT_LOG_ERROR)
target_sources(CobaltCore PUBLIC ${COBALT_CORE_HEADERS} PRIVATE ${COBALT_CORE_SOURCES})
target_include_directories(CobaltCore BEFORE PUBLIC ${COBALT_ROOT_DIR} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(CobaltCore PUBLIC CobaltMath)