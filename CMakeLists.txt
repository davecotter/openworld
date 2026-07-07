cmake_minimum_required(VERSION 3.24)
project(openworld-sos VERSION 0.1.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(SOS_BUILD_TESTS "Build sos-core unit tests" ON)

if(SOS_BUILD_TESTS)
  enable_testing()
endif()

add_subdirectory(core)
