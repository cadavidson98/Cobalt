set(SHADER_TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)

find_package(OSL REQUIRED)

message(STATUS "Adding my one off test")

add_executable(ShaderTest ${SHADER_TEST_SOURCES})

target_link_libraries(ShaderTest PRIVATE OSL::oslcomp OSL::oslexec)