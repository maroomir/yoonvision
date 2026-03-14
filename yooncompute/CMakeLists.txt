project(yooncompute CXX)

set(CMAKE_CXX_STANDARD 17)

# pkgconfig 설정
include(GNUInstallDirs)
set(PREFIX ${CMAKE_INSTALL_PREFIX})
set(LIBDIR ${CMAKE_INSTALL_LIBDIR}) # GNUInstallDirs에서 가져옴
set(INCLUDEDIR "${PREFIX}/${CMAKE_INSTALL_INCLUDEDIR}")
if(NOT DEFINED VERSION OR VERSION STREQUAL "")
  set(VERSION "${CMAKE_PROJECT_VERSION}")
endif()

# 소스 파일 추가
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
file(GLOB_RECURSE YOONCOMPUTE_SRC
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

# ONNX Runtime 찾기 (Homebrew 등)
find_path(ONNXRUNTIME_INCLUDE_DIR onnxruntime_cxx_api.h
  PATHS /opt/homebrew/include /usr/local/include
  PATH_SUFFIXES onnxruntime onnxruntime/core/session
)

find_library(ONNXRUNTIME_LIBRARY NAMES onnxruntime
  PATHS /opt/homebrew/lib /usr/local/lib
)

if(NOT ONNXRUNTIME_INCLUDE_DIR OR NOT ONNXRUNTIME_LIBRARY)
  message(FATAL_ERROR "ONNX Runtime not found. Please install via: brew install onnxruntime")
endif()

message(STATUS "Found ONNX Runtime Include: ${ONNXRUNTIME_INCLUDE_DIR}")
message(STATUS "Found ONNX Runtime Library: ${ONNXRUNTIME_LIBRARY}")

# Shared Library 생성
add_library(${PROJECT_NAME} SHARED ${YOONCOMPUTE_SRC})

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${ONNXRUNTIME_INCLUDE_DIR})

target_link_libraries(${PROJECT_NAME} PUBLIC
    ${ONNXRUNTIME_LIBRARY})

# 설치 설정
install(TARGETS ${PROJECT_NAME}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY
  include/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

configure_file(
  lib${PROJECT_NAME}.pc.in
  lib${PROJECT_NAME}.pc
  @ONLY
)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/lib${PROJECT_NAME}.pc
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)
