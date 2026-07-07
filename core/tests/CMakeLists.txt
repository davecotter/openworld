cmake_minimum_required(VERSION 3.24)

add_executable(sos_core_smoke_test smoke_test.cpp)
target_link_libraries(sos_core_smoke_test PRIVATE sos_core)

add_test(NAME sos_core_smoke COMMAND sos_core_smoke_test)
