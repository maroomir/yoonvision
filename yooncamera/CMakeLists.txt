project(yooncamera LANGUAGES CXX)

# pkgconfig 설정
include(GNUInstallDirs)
set(PREFIX ${CMAKE_INSTALL_PREFIX})
set(LIBDIR ${CMAKE_INSTALL_LIBDIR}) # GNUInstallDirs에서 가져옴
set(INCLUDEDIR "${PREFIX}/${CMAKE_INSTALL_INCLUDEDIR}")
if(NOT DEFINED VERSION OR VERSION STREQUAL "")
  set(VERSION "${CMAKE_PROJECT_VERSION}")
endif()

# 소스 파일 추가
file(GLOB YOONCAMERA_SOURCES CONFIGURE_DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

# Shared Library 생성
add_library(${PROJECT_NAME} SHARED ${YOONCAMERA_SOURCES})
target_include_directories(${PROJECT_NAME}
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# 플랫폼별 구현체 추가
if(APPLE)
  add_subdirectory(avfcam)
endif()

# Tests 포함
if(BUILD_TESTS)
  add_subdirectory(test)
endif()

# 설치
install(TARGETS ${PROJECT_NAME}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
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

install(DIRECTORY
  ${CMAKE_CURRENT_SOURCE_DIR}/include/
  DESTINATION include
)
