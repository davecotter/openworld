cmake_minimum_required(VERSION 3.24)
project(sos_quest LANGUAGES CXX)

set(SOS_CORE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../../core")
set(SOS_CORE_BUILD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../build/quest/core")

add_library(sos_core STATIC IMPORTED)
set_target_properties(sos_core PROPERTIES IMPORTED_LOCATION "${SOS_CORE_BUILD_DIR}/libsos_core.a")
target_include_directories(sos_core INTERFACE "${SOS_CORE_DIR}/include")

add_library(sos_quest SHARED
  sos_quest_main.cpp
  SosCoreBridge.cpp
  render/GlesSplatRenderer.cpp
)

target_include_directories(sos_quest PRIVATE
  "${SOS_CORE_DIR}/include"
  "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(sos_quest
  sos_core
  GLESv3
  EGL
  android
  log
)
