cmake_minimum_required(VERSION 2.8.7)
project(png2svg)
find_package(PkgConfig REQUIRED)

set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} "-std=c11 -Wall")
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-Wvla -Wall")

add_executable(
    png2svg
    png2svg.cpp
    lodepng.c)
