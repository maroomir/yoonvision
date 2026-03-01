cmake_minimum_required(VERSION 3.14)

include(FetchContent)

find_package(GTest QUIET)
if(NOT GTest_FOUND)
  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip)
  FetchContent_MakeAvailable(googletest)
endif()

add_executable(yoonvision-test test_image.cpp)
target_link_libraries(yoonvision-test PRIVATE yoonvision GTest::gtest_main)
target_compile_definitions(yoonvision-test PRIVATE YOONVISION_PROJECT_DIR="${CMAKE_SOURCE_DIR}")

include(GNUInstallDirs)
install(TARGETS yoonvision-test
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

include(GoogleTest)
gtest_discover_tests(yoonvision-test)