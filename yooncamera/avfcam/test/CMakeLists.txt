cmake_minimum_required(VERSION 3.14)

include(FetchContent)

find_package(GTest QUIET)
if(NOT GTest_FOUND AND NOT TARGET GTest::gtest_main)
  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip)
  FetchContent_MakeAvailable(googletest)
endif()

add_executable(yooncamera-avfcam-test
  test_avf_camera_context_manager.cpp
  test_avf_camera_singleshot.cpp
  test_avf_camera_streaming.cpp)

target_include_directories(yooncamera-avfcam-test
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(yooncamera-avfcam-test
  PRIVATE
    yooncamera_avfcam
    yooncamera
    yoonvision
    GTest::gtest_main)

include(GNUInstallDirs)
install(TARGETS yooncamera-avfcam-test
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

include(GoogleTest)
gtest_discover_tests(yooncamera-avfcam-test)
