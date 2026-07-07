cmake_minimum_required(VERSION 3.24)

add_library(sos_core STATIC
  src/hash.cpp
  src/room_record.cpp
  src/zip_io.cpp
  src/splat_asset.cpp
  src/splat_decode.cpp
  src/mesh_proxy_generator.cpp
  src/local_swm_store.cpp
  src/splat_import_ply.cpp
  src/normalization_pipeline.cpp
  src/room_bundle_io.cpp
  src/sos_c_api.cpp
)

target_include_directories(sos_core
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_definitions(sos_core PRIVATE SOS_CORE_BUILD)

if(SOS_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
