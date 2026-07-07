# Canonical Qt install on this machine (override with -DQT_ROOT=... or env QT_ROOT).
if(NOT QT_ROOT)
  if(DEFINED ENV{QT_ROOT})
    set(QT_ROOT "$ENV{QT_ROOT}")
  else()
    set(QT_ROOT "/Users/davec/Developer/Qt")
  endif()
endif()

set(QT_VERSION "6.11.1" CACHE STRING "Qt version directory under QT_ROOT")
set(QT_MACOS "${QT_ROOT}/${QT_VERSION}/macos")
set(QT_CREATOR_APP "${QT_ROOT}/Qt Creator.app")
set(QT_CMAKE "${QT_ROOT}/Tools/CMake/CMake.app/Contents/bin/cmake")
set(QT_NINJA "${QT_ROOT}/Tools/Ninja/ninja")

message(STATUS "QT_ROOT=${QT_ROOT}")
message(STATUS "QT_MACOS=${QT_MACOS}")
