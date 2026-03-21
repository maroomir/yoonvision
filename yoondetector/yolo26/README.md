# YOLO26 Object Detection

YOLO26 기반의 객체 감지 C++ 구현입니다. ONNX Runtime을 사용하여 실시간 객체 감지를 수행합니다.

## 개요

YOLO26는 Ultralytics에서 개발한 최신 객체 감지 모델로, 엣지 디바이스와 저전력 환경에 최적화되어 있습니다. 이 패키지는 YOLO26 모델을 ONNX 형식으로 변환하여 C++ 환경에서 효율적으로 사용할 수 있는 인터페이스를 제공합니다.

## 주요 특징

- **NMS-Free 추론**: 일대일 헤드를 사용하여 별도의 후처리(NMS) 없이 종단 간(end-to-end) 추론
- **CPU 최적화**: YOLO11 대비 최대 43% 더 빠른 CPU 추론 성능
- **DFL 제거**: 모델 간소화로 엣지 디바이스 호환성 향상
- **GPU 지원**: ONNX Runtime CUDA를 통한 GPU 가속 지원
- **다양한 모델 크기**: Nano에서 Extra Large까지 다양한 사이즈 제공

## 디렉토리 구조

```
humanoid_object_detection/yolo26/
├── CMakeLists.txt                    # 빌드 설정
├── README.md                         # 이 파일
├── humanoid_object_detection_yolo26.pc.in
├── include/
│   └── humanoid_object_detection/
│       └── yolo26/
│           ├── yolo26_detector.hpp   # 디텍터 클래스
│           ├── yolo26_types.hpp      # 타입 정의
│           ├── yolo26_preprocessor.hpp
│           └── yolo26_postprocessor.hpp
├── src/
│   ├── yolo26_detector.cpp
│   ├── yolo26_preprocessor.cpp
│   └── yolo26_postprocessor.cpp
├── models/                           # ONNX 모델 배치 디렉토리
├── python/
│   └── convert_yolo26_to_onnx.py     # 모델 변환 스크립트
└── test/
```

## 빠른 시작

### 1. 의존성 설치

```bash
# Ultralytics 설치 (모델 변환용)
pip install ultralytics
```

### 2. 모델 변환

YOLO26 PyTorch 모델을 ONNX로 변환합니다:

```bash
cd humanoid_object_detection/yolo26/python

# 기본 설정 (Nano 모델)
python convert_yolo26_to_onnx.py
```

**중요**: 모델 파일이 없으면 Ultralytics가 자동으로 다운로드합니다. 첫 실행 시 다운로드가 필요하며, 다운로드된 모델은 현재 디렉토리에 저장됩니다.

다른 모델 사용:
```bash
python convert_yolo26_to_onnx.py --model yolo26s.pt  # Small
python convert_yolo26_to_onnx.py --model yolo26m.pt  # Medium
python convert_yolo26_to_onnx.py --model yolo26l.pt  # Large
python convert_yolo26_to_onnx.py --model yolo26x.pt  # Extra Large
```

### 3. 모델 배치

변환된 ONNX 모델을 `models/` 디렉토리로 이동:

```bash
mv yolo26n.onnx ../models/
```

### 4. 빌드

```bash
# 루트 디렉토리에서
cd ../../
./build.sh ENABLE_YOLO26=on
```

### 5. 사용 예시

```cpp
#include "humanoid_object_detection/yolo26/yolo26_detector.hpp"

using namespace kbot::humanoid_object_detection::yolo26;

// 디텍터 생성
YOLO26Detector detector;

// 설정
YOLO26Config config;
config.model_path = "models/yolo26n.onnx";
config.confidence_threshold = 0.5f;
config.iou_threshold = 0.45f;
config.use_one_to_one_head = true;  // NMS-free 모드
config.gpu_device_id = 0;           // GPU 사용 시, -1이면 CPU

// 초기화
if (!detector.Initialize(config)) {
    std::cerr << "Failed to initialize detector" << std::endl;
    return -1;
}

// 감지 수행
cv::Mat image = cv::imread("test.jpg");
DetectionResult result = detector.Detect(image);

// 결과 확인
std::cout << "Detected " << result.detections.size() << " objects" << std::endl;
std::cout << "Inference time: " << result.inference_time_ms << " ms" << std::endl;

// 시각화
cv::imshow("Detection", result.visualization);
cv::waitKey(0);
```

## YOLO26 모델 변환 가이드

### 자동 다운로드

YOLO26 모델은 Ultralytics에서 공식적으로 제공됩니다. 모델 파일이 로컬에 없는 경우, 첫 번째 `YOLO()` 호출 시 자동으로 다운로드됩니다. 다운로드된 모델은 현재 작업 디렉토리에 저장됩니다.

**다운로드 소스**: https://github.com/ultralytics/assets/releases/download/v8.4.0/

**주의**: 모델 다운로드에는 인터넷 연결이 필요합니다. 첫 실행 시 약간의 시간이 소요될 수 있습니다.

### 변환 옵션

`convert_yolo26_to_onnx.py` 스크립트는 다음 옵션을 지원합니다:

| 옵션 | 기본값 | 설명 |
|------|--------|------|
| `--model` | `yolo26n.pt` | YOLO26 모델 경로 |
| `--imgsz` | `640` | 입력 이미지 크기 |
| `--no-simplify` | false | ONNX 모델 간소화 비활성화 |
| `--half` | false | FP16 정밀도 사용 |
| `--one-to-many` | false | 일대다 헤드 사용 (NMS 필요) |

### 예제 명령어

```bash
# 기본 변환 (Nano 모델, 640x640, 일대일 헤드)
python convert_yolo26_to_onnx.py

# Small 모델, FP16, 1280x1280
python convert_yolo26_to_onnx.py --model yolo26s.pt --half --imgsz 1280

# 일대다 헤드 (NMS 필요, 약간 더 높은 정확도)
python convert_yolo26_to_onnx.py --one-to-many

# 모델 간소화 비활성화 (디버깅용)
python convert_yolo26_to_onnx.py --no-simplify
```

### 모델 비교

| 모델 | 파라미터 | FLOPs | CPU 추론 시간 | T4 GPU 시간 | 정확도 (mAP) |
|------|----------|-------|---------------|-------------|--------------|
| yolo26n | 2.4M | 5.4B | ~40ms | ~2ms | 40.9 |
| yolo26s | 9.5M | 20.7B | ~90ms | ~3ms | 48.6 |
| yolo26m | 20.4M | 68.2B | ~230ms | ~5ms | 53.1 |
| yolo26l | 24.8M | 86.4B | ~290ms | ~6ms | 55.0 |
| yolo26x | 55.7M | 193.9B | ~530ms | ~12ms | 57.5 |

### 헤드 선택 가이드

**일대일 헤드 (기본값, 권장)**
- ✅ NMS 없이 종단 간 추론
- ✅ 더 빠른 추론 속도
- ✅ 간단한 배포
- ✅ 엣지 디바이스에 최적화
- 사용: `--one-to-many` 플래그 없이 실행

**일대다 헤드**
- ⚠️ NMS 후처리 필요
- 약간 더 높은 정확도
- 추가 처리 오버헤드
- 사용: `--one-to-many` 플래그 추가

## 설정

### YOLO26Config

```cpp
struct YOLO26Config {
    std::string model_path = "models/yolo26n.onnx";
    float confidence_threshold = 0.5f;
    float iou_threshold = 0.45f;
    int max_detections = 300;
    bool use_one_to_one_head = true;
    int gpu_device_id = 0;  // -1: CPU, >=0: GPU device ID
    cv::Size input_size = cv::Size(640, 640);
    bool normalize = true;
    bool swap_rgb = true;  // BGR to RGB 변환
};
```

### 환경 변수

| 변수 | 설명 | 기본값 |
|------|------|--------|
| `ORT_TENSORRT_ENGINE_CACHE_ENABLE` | TensorRT 엔진 시 활성화 | `0` |
| `ORT_TENSORRT_FP16_ENABLE` | TensorRT FP16 활성화 | `1` |
| `CUDA_VISIBLE_DEVICES` | 사용할 GPU 디바이스 | `0` |

## 성능 최적화

### GPU 사용

```cpp
YOLO26Config config;
config.gpu_device_id = 0;  // GPU 0 사용
```

### TensorRT 사용 (추천)

TensorRT는 ONNX Runtime의 TensorRT Execution Provider를 통해 자동으로 사용됩니다:

```bash
export ORT_TENSORRT_ENGINE_CACHE_ENABLE=1
export ORT_TENSORRT_FP16_ENABLE=1
```

### 배치 추론

여러 이미지를 동시에 처리:

```cpp
std::vector<cv::Mat> images = {img1, img2, img3};
std::vector<DetectionResult> results = detector.DetectBatch(images);
```

## 트러블슈팅

### 모델 다운로드/로드 실패

**문제**: 모델 파일을 찾을 수 없음
```bash
⚠️  Model file not found: yolo26n.pt
```

**해결**:
Ultralytics가 자동으로 모델을 다운로드합니다. 인터넷 연결을 확인하고 다시 실행하세요. 다운로드는 Ultralytics의 공식 저장소(https://github.com/ultralytics/assets/releases/)에서 진행됩니다.

### ONNX 모델 로드 실패

**문제**: ONNX 모델 파일을 찾을 수 없음
```bash
❌ Error: Model file not found: models/yolo26n.onnx
```

**해결**:
1. 모델 변환 완료 확인
2. ONNX 파일이 올바른 위치에 있는지 확인:
```bash
ls -la humanoid_object_detection/yolo26/models/
```
3. 변환된 ONNX 파일을 models/ 디렉토리로 이동:
```bash
cd humanoid_object_detection/yolo26/python
mv yolo26n.onnx ../models/
```

### GPU 메모리 부족

**문제**: CUDA 메모리 할당 실패

**해결**:
1. 더 작은 모델 사용 (yolo26n 또는 yolo26s)
2. 입력 크기 감소: `config.input_size = cv::Size(416, 416)`
3. CPU 모드 사용: `config.gpu_device_id = -1`

### 추론 속도 느림

**해결**:
1. TensorRT 활성화
2. FP16 사용: `python convert_yolo26_to_onnx.py --half`
3. 작은 모델 사용
4. 일대일 헤드 사용

## 의존성

- **OpenCV**: 이미지 처리
- **ONNX Runtime**: 모델 추론
- **humanoid_compute**: ONNX Runtime GPU 래퍼

## 참고 자료

- [Ultralytics YOLO26 문서](https://docs.ultralytics.com/)
- [ONNX Runtime C++ API](https://onnxruntime.ai/docs/api/c/)
- [YOLO26 계획 문서](../../YOLO26_PLAN.md)

## 라이선스

Samsung Research
