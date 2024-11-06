add_library(CobaltCore STATIC)
target_compile_features(CobaltCore PUBLIC cxx_std_20)

add_subdirectory(math)
add_subdirectory(geometry)
add_subdirectory(material)
add_subdirectory(tests)
add_subdirectory(utilities)

target_include_directories(CobaltCore BEFORE PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(CobaltCore PRIVATE)