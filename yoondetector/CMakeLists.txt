project(yoondetector CXX)

set(CMAKE_CXX_STANDARD 17)

include(GNUInstallDirs)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

file(GLOB_RECURSE YOONDETECTOR_SRC
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

add_library(${PROJECT_NAME} SHARED ${YOONDETECTOR_SRC})

target_include_directories(${PROJECT_NAME}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME}
  PUBLIC
    yoonvision
    yooncompute
)

install(TARGETS ${PROJECT_NAME}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY
  ${CMAKE_CURRENT_SOURCE_DIR}/include/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

add_subdirectory(yolo26)

