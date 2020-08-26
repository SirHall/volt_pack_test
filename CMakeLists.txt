cmake_minimum_required(VERSION 3.7)

project(volt_pack_tester)

# Add in volt_pack
add_library(volt_pack INTERFACE)
target_include_directories(volt_pack INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/volt_pack)

enable_testing()

# Add in tests
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/tests/)
