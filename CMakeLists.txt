cmake_minimum_required(VERSION 3.10)

project(anthracite)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_custom_target(build-version
    COMMAND cd ../build_supp && ./version.sh
    DEPENDS build_supp/version.txt 
    COMMENT "Generated supplemental build files (version)"
)

add_custom_target(build-supplemental 
    COMMAND cd ../build_supp && python3 ./error_gen.py
    COMMAND cp -r ../www .
    DEPENDS build_supp/version.txt www/* build_supp/error_gen.py build-version
    COMMENT "Generated supplemental build files (default www dir + error pages)"
)

add_custom_target(lint
    COMMAND clang-tidy *.cpp 
)

add_custom_target(lint-fix
    COMMAND clang-tidy *.cpp -fix -fix-errors 
)

add_custom_target(build-docker
    COMMAND docker build .. -t anthracite
)

add_custom_target(run
    COMMAND anthracite-bin 
    DEPENDS anthracite-bin 
    WORKING_DIRECTORY ${CMAKE_PROJECT_DIR}
)

FILE(GLOB LIB_SOURCES lib/*.cpp lib/**/*.cpp build_supp/version.cpp)
add_library(anthracite ${LIB_SOURCES}) 
add_dependencies(anthracite build-version)

add_executable(anthracite-bin src/file_main.cpp) 
target_link_libraries(anthracite-bin anthracite)
add_dependencies(anthracite-bin build-supplemental)
add_dependencies(anthracite-bin anthracite)

add_executable(anthracite-api-bin src/api_main.cpp) 
target_link_libraries(anthracite-api-bin anthracite)

