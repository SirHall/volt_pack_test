cmake_minimum_required(VERSION 3.7)

add_executable(include_finder ${CMAKE_CURRENT_SOURCE_DIR}/include_finder.cpp)
target_link_libraries(include_finder volt_pack)
set_property(TARGET include_finder PROPERTY CXX_STANDARD 17)

add_test(IncludeFinder include_finder)