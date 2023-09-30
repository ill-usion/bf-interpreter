cmake_minimum_required(VERSION 3.0.0)
project(bf-interpreter VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
add_compile_options(-Wall -Wpedantic -Wconversion -Werror -g0 -O3)
add_executable(bf-interpreter main.cpp)
