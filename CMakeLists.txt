cmake_minimum_required(VERSION 3.14)
project(yoonvision VERSION 0.1.0 LANGUAGES C CXX)

# 빌드 설정
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_TESTS "Build test target" ON)

# CMake preset / 설치 경로 설정
include(GNUInstallDirs)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/preset.cmake)

# macOS 동적 라이브러리 탐색 경로(RPATH) 설정
if(APPLE)
  set(CMAKE_MACOSX_RPATH ON)
  set(CMAKE_SKIP_BUILD_RPATH FALSE)
  set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
  set(CMAKE_BUILD_RPATH
    "${CMAKE_BINARY_DIR};${CMAKE_BINARY_DIR}/yooncamera;${CMAKE_BINARY_DIR}/yooncamera/avfcam"
  )
  set(CMAKE_INSTALL_RPATH "@loader_path/../lib;@executable_path/../lib")
  set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
endif()

# 의존성 설정
include(FindPkgConfig)
pkg_check_modules(JPEG REQUIRED libjpeg)

# 헤더 경로 설정
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# 소스 파일 추가
file(GLOB_RECURSE YOONVISION_SOURCES CONFIGURE_DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

# 테스트 설정
if(BUILD_TESTS)
  enable_testing()
endif()

# 라이브러리 빌드
add_library(${PROJECT_NAME} ${YOONVISION_SOURCES})

target_include_directories(
  ${PROJECT_NAME}
  PUBLIC
    ${JPEG_INCLUDE_DIRS}
)

target_link_directories(${PROJECT_NAME}
  PUBLIC
    ${JPEG_LIBRARY_DIRS}
)
target_link_libraries(${PROJECT_NAME}
  PUBLIC
    ${JPEG_LIBRARIES}
)

# 하위 모듈 빌드
add_subdirectory(yooncamera)

if(BUILD_TESTS)
  add_subdirectory(test)
endif()

# 설치
install(TARGETS ${PROJECT_NAME}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY
  include/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)