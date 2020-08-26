cmake_minimum_required(VERSION 3.7)

# Copy test's resource files to ouput
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/res DESTINATION ${UNIT_TEST_BIN_OUTPUT_DIR}/)

add_executable(include_finder ${CMAKE_CURRENT_SOURCE_DIR}/include_finder.cpp)
target_link_libraries(include_finder volt_pack)
set_property(TARGET include_finder PROPERTY CXX_STANDARD 17)

add_test(include_finder include_finder)