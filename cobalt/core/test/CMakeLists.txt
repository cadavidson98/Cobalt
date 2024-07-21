enable_testing()

find_package(GTest)
if (NOT ${GTEST_LIBRARY})
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
endif()

add_executable(CobaltCoreTests testMath.cpp testGeometry.cpp)
target_link_libraries(CobaltCoreTests PUBLIC CobaltMath CobaltGeometry GTest::gtest_main)
add_test(TestCobaltCore CobaltCoreTests)