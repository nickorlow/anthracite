cmake_minimum_required(VERSION 3.10)

project(anthracite)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(OpenSSL REQUIRED)

configure_file(build_supp/version.txt version.txt COPYONLY)

add_custom_command(
    COMMAND ../build_supp/version.sh
    DEPENDS version.txt 
    OUTPUT ${CMAKE_BINARY_DIR}/version.cpp
    COMMENT "Generated supplemental build files (version)"
)

add_custom_target(build-supplemental 
    COMMAND cd ../build_supp && python3 ./error_gen.py
    COMMAND mkdir -p www && cp -r ../default_www/regular/* ./www/
    COMMAND cp ../build_supp/default_config.cfg ./anthracite.cfg
    DEPENDS ../default_www/regular/* build_supp/error_gen.py ${CMAKE_BINARY_DIR}/version.cpp
    COMMENT "Generated supplemental build files (default www dir + default config + error pages)"
)

FILE(GLOB LIB_SOURCES lib/*.cpp lib/**/*.cpp)
add_library(anthracite ${LIB_SOURCES} ${CMAKE_BINARY_DIR}/version.cpp) 
add_dependencies(anthracite build-supplemental)
target_link_libraries(anthracite OpenSSL::SSL OpenSSL::Crypto)
target_include_directories(anthracite PUBLIC ${OPENSSL_INCLUDE_DIR})

add_executable(anthracite-bin src/file_main.cpp) 
target_link_libraries(anthracite-bin anthracite)
add_dependencies(anthracite-bin build-supplemental)
add_dependencies(anthracite-bin anthracite)

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
