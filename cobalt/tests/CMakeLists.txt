enable_testing()

find_package(GTest REQUIRED)

# add test data
set(CobaltTestsDataDir ${CMAKE_CURRENT_SOURCE_DIR}/TestData)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/resources.h.in ${CMAKE_CURRENT_SOURCE_DIR}/resources.h)

add_executable(CobaltTests testMath.cpp testGeometry.cpp testMaterial.cpp testUtilities.cpp resources.h)
add_subdirectory(render)
target_include_directories(CobaltTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(CobaltTests PUBLIC Cobalt GTest::gtest_main GTest::gtest)