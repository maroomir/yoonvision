cmake_minimum_required(VERSION 3.14)
project(yoonvision VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_TESTS "Build test target" ON)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/preset.cmake)

include(FindPkgConfig)
pkg_check_modules(JPEG REQUIRED libjpeg)

file(GLOB_RECURSE YOONVISION_SOURCES CONFIGURE_DEPENDS src/*.cpp)

add_library(yoonvision ${YOONVISION_SOURCES})
add_library(yoonimage ALIAS yoonvision)

target_include_directories(
  yoonvision
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${JPEG_INCLUDE_DIRS})

target_link_directories(yoonvision PUBLIC ${JPEG_LIBRARY_DIRS})
target_link_libraries(yoonvision PUBLIC ${JPEG_LIBRARIES})

if(BUILD_TESTS)
  enable_testing()
  add_subdirectory(test)
endif()

include(GNUInstallDirs)
install(TARGETS yoonvision
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})