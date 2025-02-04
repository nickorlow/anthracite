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
    COMMAND mkdir -p www && cp -r ../default_www/regular/* ./www/
    DEPENDS build_supp/version.txt ../default_www/regular/* build_supp/error_gen.py build-version
    COMMENT "Generated supplemental build files (default www dir + error pages)"
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

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

FetchContent_MakeAvailable(googletest)

file(GLOB TESTS_SRC CONFIGURE_DEPENDS "tests/*.cpp")
enable_testing()

add_custom_target(test_files
    COMMAND cp -r ../tests/test_files . 
    DEPENDS ../tests/test_files/* 
    COMMENT "Copied test resource files"
)

add_executable(
  tests
  ${TESTS_SRC}
)
add_dependencies(tests anthracite)
add_dependencies(tests test_files)

target_link_libraries(
  tests
  GTest::gtest_main
)
target_link_libraries(
  tests
  anthracite
)

include(GoogleTest)
gtest_discover_tests(tests)
