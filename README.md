# yoonvision

ROS2 없이 CMake 기반으로 구성된 C++ 비전/카메라 프로젝트입니다.

## 구성 요소

- `yoonvision`  
  기본 이미지 데이터 구조 및 이미지 처리 라이브러리
- `yooncamera`  
  카메라 인터페이스/팩토리/스트림 추상화 라이브러리
- `yoonvision_http`  
  이미지 스트림 HTTP 퍼블리싱/렌더링 라이브러리
- `yoonvision_viewer`  
  카메라 스트림을 웹으로 확인하는 실행 애플리케이션

> 참고: `yooncamera/avfcam` 구현은 macOS(AVFoundation) 전용입니다.

## 요구 사항

- CMake 3.14+
- C++17 컴파일러
- `pkg-config`
- `libjpeg` (`pkg-config`의 `libjpeg` 모듈로 탐색)

## 빠른 빌드

기본 빌드(Configure + Build + Install):

```bash
./build.sh
```

명시적으로 `cmake` 모드 실행:

```bash
./build.sh cmake
```

`build.sh`는 내부적으로 아래 순서로 수행합니다.

1. `cmake -S . -B build ...`
2. `cmake --build build --target all -- -j <jobs>`
3. `cmake --install build`

## build.sh 옵션

```bash
./build.sh [cmake|clean|help]
```

- `cmake`: Configure + Build + Install
- `clean`: `build/`, `install/` 디렉터리 삭제
- `help`: 도움말 출력

## build.sh 환경 변수

- `OS_NAME` (기본값: `macos`)
  - `macos`
  - `linux_x86_64`
  - `window_x86_64`
- `BUILD_TYPE` (기본값: `Release`)
- `BUILD_TESTS` (기본값: `ON`)

예시:

```bash
BUILD_TYPE=Debug ./build.sh cmake
OS_NAME=linux_x86_64 ./build.sh cmake
BUILD_TESTS=OFF ./build.sh cmake
```

플랫폼 매크로 설정은 `cmake/preset.cmake`에서 관리됩니다.

## CMake 옵션

루트 `CMakeLists.txt`에서 아래 옵션을 제공합니다.

- `BUILD_TESTS` (기본값: `ON`)
- `BUILD_HTTP_VIEWER` (기본값: `ON`)
- `BUILD_VIEWER` (기본값: `ON`)

직접 CMake를 사용할 때 예시:

```bash
cmake -S . -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=./install \
  -DBUILD_TESTS=ON \
  -DBUILD_HTTP_VIEWER=ON \
  -DBUILD_VIEWER=ON

cmake --build build --target all -- -j 8
cmake --install build
```

## 테스트

`BUILD_TESTS=ON`으로 빌드한 뒤:

```bash
ctest --test-dir build --output-on-failure
```

주요 테스트 타겟:

- `yoonvision-test`
- `yooncamera-test`

## 뷰어 실행

`yoonvision_viewer` 실행 시 기본적으로 카메라 타입 `avfcam`을 사용하며,
HTTP 서버를 포트 `8080`으로 초기화합니다.

예시:

```bash
./build/yoonvision_viewer/yoonvision_viewer
```

실행 후 브라우저에서 `http://localhost:8080`으로 접속해 스트림을 확인합니다.

## 설치 산출물

설치 시 라이브러리/헤더와 함께 일부 모듈은 pkg-config 파일을 설치합니다.

- `yooncamera`: `libyooncamera.pc`
- `yoonvision_http`: `libyoonvision_http.pc`

## 정리

```bash
./build.sh clean
```

