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
set(YOONVISION_TEST_TARGET yoonvision-test)

add_executable(${YOONVISION_TEST_TARGET}
  test_image.cpp
)

# 테스트 타겟 설정
target_link_libraries(${YOONVISION_TEST_TARGET}
  PRIVATE
    yoonvision
    GTest::gtest_main
)
target_compile_definitions(${YOONVISION_TEST_TARGET}
  PRIVATE
    YOONVISION_PROJECT_DIR="${CMAKE_SOURCE_DIR}"
)

# 설치
include(GNUInstallDirs)
install(TARGETS ${YOONVISION_TEST_TARGET}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# 테스트 등록
include(GoogleTest)
gtest_discover_tests(${YOONVISION_TEST_TARGET})