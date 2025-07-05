enable_testing()

find_package(GTest REQUIRED)

# add test data
set(CobaltTestsDataDir ${CMAKE_CURRENT_SOURCE_DIR}/TestData)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/resources.h.in ${CMAKE_CURRENT_SOURCE_DIR}/resources.h)

set(COBALT_TEST_SOURCES
    resources.h
    testAlgorithms.cpp
    testGeometry.cpp
    testMath.cpp
    testMaterial.cpp
    testString.cpp
    testUtilities.cpp
)

add_executable(CobaltTests ${COBALT_TEST_SOURCES})

add_subdirectory(render)
target_include_directories(CobaltTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(CobaltTests PUBLIC Cobalt GTest::gtest_main GTest::gtest)