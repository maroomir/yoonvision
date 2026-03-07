project(yooncamera_avfcam LANGUAGES CXX OBJCXX)

# AVFoundation은 macOS 전용
if(NOT APPLE)
  message(FATAL_ERROR "yooncamera_avfcam requires macOS (AVFoundation)")
endif()

# 소스 파일 추가
file(GLOB YOONCAMERA_AVFCAM_SOURCES CONFIGURE_DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.mm
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

# Shared Library 생성
add_library(${PROJECT_NAME} SHARED ${YOONCAMERA_AVFCAM_SOURCES})

# 인클루드 경로 설정
target_include_directories(${PROJECT_NAME}
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
  PRIVATE
    # yoonvision 루트 include (image.hpp 등)
    ${CMAKE_SOURCE_DIR}/include
    # yooncamera 공용 include (camera.hpp 등)
    ${CMAKE_SOURCE_DIR}/yooncamera/include
)

# 의존 라이브러리 링크
target_link_libraries(${PROJECT_NAME}
  PUBLIC
    yooncamera
    yoonvision
  PRIVATE
    "-framework AVFoundation"
    "-framework CoreMedia"
    "-framework CoreVideo"
    "-framework Foundation"
)

# Objective-C++ 컴파일 옵션
target_compile_options(${PROJECT_NAME} PRIVATE
  -fobjc-arc         # Automatic Reference Counting 활성화
)

# 설치
include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY
  ${CMAKE_CURRENT_SOURCE_DIR}/include/
  DESTINATION include
)

# Tests 포함
if(BUILD_TESTS)
  add_subdirectory(test)
endif()
