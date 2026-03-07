cmake_minimum_required(VERSION 3.14)

# GTest 의존성 설정
include(FetchContent)

find_package(GTest QUIET)
if(NOT GTest_FOUND AND NOT TARGET GTest::gtest_main)
  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
  )
  FetchContent_MakeAvailable(googletest)
endif()

# 테스트 실행 파일 생성
set(YOONCAMERA_TEST_TARGET yooncamera-test)

add_executable(${YOONCAMERA_TEST_TARGET}
  test_camera_factory.cpp
  test_mock_camera.cpp
  test_mock_camera_context_manager.cpp
  test_camera_stream.cpp
)

# 테스트 타겟 설정
target_include_directories(${YOONCAMERA_TEST_TARGET}
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(${YOONCAMERA_TEST_TARGET}
  PRIVATE
    yooncamera
    GTest::gtest_main
    GTest::gmock_main
)

# 설치
include(GNUInstallDirs)
install(TARGETS ${YOONCAMERA_TEST_TARGET}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# 테스트 등록
include(GoogleTest)
gtest_discover_tests(${YOONCAMERA_TEST_TARGET})
