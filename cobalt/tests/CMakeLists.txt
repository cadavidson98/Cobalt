enable_testing()

find_package(GTest REQUIRED)

# add test data
set(CobaltTestsDataDir ${CMAKE_CURRENT_SOURCE_DIR}/TestData)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/resources.h.in ${CMAKE_CURRENT_SOURCE_DIR}/resources.h)

set(COBALT_TEST_SOURCES
    resources.h
    test_algorithms.cpp
    test_color.cpp
    test_geometry.cpp
    test_math.cpp
    test_material.cpp
    test_string.cpp
    test_utilities.cpp
)

add_executable(CobaltTests ${COBALT_TEST_SOURCES})

add_subdirectory(render)
target_include_directories(CobaltTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(CobaltTests PUBLIC Cobalt GTest::gtest_main GTest::gtest)