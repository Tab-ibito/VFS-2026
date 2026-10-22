cmake_minimum_required(VERSION 3.16)

project(code LANGUAGES CXX)

add_executable(code
    ../main.cpp
    ../file_sys.hpp
)

set_target_properties(code PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)
