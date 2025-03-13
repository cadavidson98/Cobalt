cmake_minimum_required(VERSION 3.23)
project(Cobalt LANGUAGES C CXX)

# config options here

# add cobalt renderer
add_subdirectory(cobalt)
# add tools
add_subdirectory(tools)