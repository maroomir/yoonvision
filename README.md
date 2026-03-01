# yoonvision

ROS2 없이 CMake 기반으로 빌드하는 이미지 라이브러리 프로젝트입니다.

## 요구 사항

- CMake 3.10+
- C++17 컴파일러
- `libjpeg` (pkg-config로 탐색)

## 빌드 (cmake_build 기준)

`amr_vision`의 `cmake_build` 흐름과 유사하게 `build.sh`를 제공합니다.

```bash
./build.sh cmake
```

위 명령은 아래를 순서대로 수행합니다.

1. Configure: `cmake -S . -B build ...`
2. Build: `cmake --build build --target all -- -j <jobs>`
3. Install: `cmake --install build`

인자 없이 실행하면 기본값으로 `cmake` 모드가 동작합니다.

```bash
./build.sh
```

OS 설정은 실행 인자가 아니라 `build.sh` 상단의 `OS_NAME` 변수에서 관리합니다.

- `OS_NAME=macos`
- `OS_NAME=linux_x86_64`
- `OS_NAME=window_x86_64`

플랫폼/빌드모드 기본값과 해석 로직은 `cmake/preset.cmake`에 집중되어 있습니다.

## 테스트

빌드 후 테스트 실행:

```bash
ctest --test-dir build --output-on-failure
```

## 빌드 산출물 정리

```bash
./build.sh clean
```

