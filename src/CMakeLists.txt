cmake_minimum_required(VERSION 3.10)

project(anthracite)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS_RELEASE "-O3")


add_custom_target(build-supplemental 
    COMMAND cd ../build_supp && ./version.sh && python3 ./error_gen.py
    COMMAND cp -r ../www .
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

FILE(GLOB SOURCES **/*.cpp)

add_library(anthracite anthracite_main.cpp ${SOURCES}) 

add_executable(anthracite-bin file_main.cpp) 
target_link_libraries(anthracite-bin anthracite)

add_executable(anthracite-api-bin api_main.cpp) 
target_link_libraries(anthracite-api-bin anthracite)

add_dependencies(anthracite-bin build-supplemental)
add_dependencies(anthracite-bin anthracite)
