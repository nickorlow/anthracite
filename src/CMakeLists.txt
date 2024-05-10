cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

project(anthracite)

add_custom_target(build-supplemental 
    COMMAND cd ./build && ./version.sh && python3 ./error_gen.py
)

add_custom_target(lint
    COMMAND clang-tidy *.cpp 
)

add_custom_target(lint-fix
    COMMAND clang-tidy *.cpp -fix -fix-errors 
)

add_custom_target(format
    COMMAND clang-format *.cpp -i
)

add_custom_target(build-docker
    COMMAND docker build .. -t anthracite
)

add_custom_target(run
    COMMAND anthracite-bin 
    DEPENDS anthracite-bin 
    WORKING_DIRECTORY ${CMAKE_PROJECT_DIR}
)

add_executable(anthracite-bin main.cpp) 
add_dependencies(anthracite-bin build-supplemental)
