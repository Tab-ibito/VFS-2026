cmake_minimum_required(VERSION 3.16)

project(vfs LANGUAGES CXX)

add_executable(vfs
    ../main.cpp
    ../file_sys.hpp
)

set_target_properties(vfs PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)
