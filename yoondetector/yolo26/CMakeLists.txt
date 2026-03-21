cmake_minimum_required(VERSION 3.16)
project(yoondetector_yolo26 VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE YOONDETECTOR_YOLO26_SRC
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

add_library(${PROJECT_NAME} SHARED ${YOONDETECTOR_YOLO26_SRC})

target_include_directories(${PROJECT_NAME}
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    ${CMAKE_CURRENT_SOURCE_DIR}/..  # 상위 yoondetector/include 접근용
)

target_link_libraries(${PROJECT_NAME}
  PUBLIC
    yoondetector
    yooncompute
)

target_compile_options(${PROJECT_NAME} PRIVATE
  -Wall
  -Wextra
  -Wpedantic
)

configure_file(
  ${PROJECT_NAME}.pc.in
  ${PROJECT_NAME}.pc
  @ONLY
)

install(TARGETS ${PROJECT_NAME}
  DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pc
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)

install(DIRECTORY
  ${CMAKE_CURRENT_SOURCE_DIR}/include/
  DESTINATION include
)

message(STATUS "yoondetector_yolo26 configuration:")
message(STATUS "  Using yooncompute for ONNX Runtime")
message(STATUS "  Using yoonvision::Image (no OpenCV dependency)")

