enable_testing()

find_package(GTest REQUIRED)
add_executable(CobaltCoreTests testMath.cpp testGeometry.cpp testMaterial.cpp testUtilities.cpp)
set_target_properties(CobaltCoreTests PROPERTIES MSVC_RUNTIME_LIBRARY MultiThreaded)
target_compile_definitions(CobaltCoreTests PUBLIC GTEST_LINKED_AS_SHARED_LIBRARY=1)
target_link_libraries(CobaltCoreTests PUBLIC CobaltCore GTest::gtest_main GTest::gtest)