cmake_minimum_required(VERSION 3.5)

project(cpp20-coroutines-threads LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(cpp20-coroutines-threads main.cpp generator.h threadPool.h)
