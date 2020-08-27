cmake_minimum_required(VERSION 3.7)

add_executable(git_download ${CMAKE_CURRENT_SOURCE_DIR}/git_download.cpp)
target_link_libraries(git_download volt_pack)
set_property(TARGET git_download PROPERTY CXX_STANDARD 17)

add_test(git_download git_download)