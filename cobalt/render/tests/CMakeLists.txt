enable_testing()

find_package(GTest REQUIRED)

# add test data
set(CobaltRenderTestsDataDir ${CMAKE_CURRENT_SOURCE_DIR}/TestData)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/resources.h.in ${CMAKE_CURRENT_SOURCE_DIR}/resources.h)

set(RENDER_TESTS ${CMAKE_CURRENT_SOURCE_DIR}/testData.cpp ${CMAKE_CURRENT_SOURCE_DIR}/resources.h)

add_executable(CobaltRenderTests ${RENDER_TESTS})
set_target_properties(CobaltRenderTests PROPERTIES MSVC_RUNTIME_LIBRARY MultiThreaded)
target_compile_definitions(CobaltRenderTests PUBLIC GTEST_LINKED_AS_SHARED_LIBRARY=1)
target_link_libraries(CobaltRenderTests PUBLIC CobaltRender GTest::gtest_main GTest::gtest)