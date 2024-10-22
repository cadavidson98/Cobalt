cmake_minimum_required(VERSION 3.9)
project(Cobalt LANGUAGES C CXX)

# config options here

# add cobalt renderer
add_subdirectory(cobalt)
# add external libraries
add_subdirectory(extern)
# add tools
add_subdirectory(tools)