cmake_minimum_required(VERSION 3.14)

include(FetchContent)

find_package(GTest QUIET)
if(NOT GTest_FOUND)
  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip)
  FetchContent_MakeAvailable(googletest)
endif()

add_executable(yooncamera-test
  test_camera_factory.cpp
  test_mock_camera.cpp
  test_mock_camera_context_manager.cpp
  test_camera_stream.cpp)

target_include_directories(yooncamera-test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(yooncamera-test PRIVATE yooncamera GTest::gtest_main GTest::gmock_main)

include(GNUInstallDirs)
install(TARGETS yooncamera-test
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

include(GoogleTest)
gtest_discover_tests(yooncamera-test)
