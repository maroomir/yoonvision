project(yoonvision_viewer LANGUAGES CXX)

# 소스 파일
file(GLOB YOONVISION_VIEWER_SOURCES CONFIGURE_DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

# 실행 파일 생성
add_executable(${PROJECT_NAME} ${YOONVISION_VIEWER_SOURCES})

# 인클루드 경로 설정
target_include_directories(${PROJECT_NAME}
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/yooncamera/include
    ${CMAKE_SOURCE_DIR}/yoonvision_http/include
)

# 빌드 타임에 HTTP 리소스 경로 정의 (소스 디렉토리 기준)
target_compile_definitions(${PROJECT_NAME}
  PRIVATE
    YOONVISION_HTTP_RESOURCES_PATH="${CMAKE_SOURCE_DIR}/yoonvision_http/resources"
)

# 링크 라이브러리
target_link_libraries(${PROJECT_NAME}
  PRIVATE
    yoonvision
    yooncamera
    yoonvision_http
)

# macOS: AVFoundation 카메라 구현체 링크
if(APPLE)
  target_link_libraries(${PROJECT_NAME} PRIVATE yooncamera_avfcam)
endif()

# 설치
install(TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
