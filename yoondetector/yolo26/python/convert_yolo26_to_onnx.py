#!/usr/bin/env python3
"""
YOLO26 모델을 ONNX로 변환하는 스크립트

사용법:
    python convert_yolo26_to_onnx.py --model yolo26n.pt --imgsz 640
"""

import argparse
from pathlib import Path
from ultralytics import YOLO


def convert_to_onnx(model_path: str, imgsz: int = 640,
                   simplify: bool = True, half: bool = False,
                   end2end: bool = True) -> str:
    """
    YOLO26 모델을 ONNX로 변환합니다.

    Args:
        model_path: YOLO26 모델 경로 (.pt)
        imgsz: 입력 이미지 크기
        simplify: ONNX 모델 간소화
        half: FP16 모델로 변환
        end2end: End-to-end NMS-free 모드 (일대일 헤드)

    Returns:
        str: 생성된 ONNX 모델 경로
    """
    print(f"Loading YOLO26 model from {model_path}...")

    # 모델 로드
    model = YOLO(model_path)

    print(f"Model loaded successfully from: {model_path}")
    print(f"Converting to ONNX...")
    print(f"  Image size: {imgsz}")
    print(f"  Simplify: {simplify}")
    print(f"  Half precision: {half}")
    print(f"  End-to-end (NMS-free): {end2end}")

    # ONNX로 내보내기
    output_path = model.export(
        format="onnx",
        imgsz=imgsz,
        simplify=simplify,
        half=half,
        dynamic=False,  # 현재는 고정 크기만 지원
        end2end=end2end  # 일대일 헤드 (NMS 없음)
    )

    print(f"\n✅ ONNX model exported to: {output_path}")
    return output_path


def main():
    parser = argparse.ArgumentParser(description="Convert YOLO26 model to ONNX")

    parser.add_argument(
        "--model",
        type=str,
        default="yolo26n.pt",
        help="YOLO26 model path (default: yolo26n.pt)"
    )

    parser.add_argument(
        "--imgsz",
        type=int,
        default=640,
        help="Input image size (default: 640)"
    )

    parser.add_argument(
        "--no-simplify",
        action="store_true",
        help="Don't simplify ONNX model"
    )

    parser.add_argument(
        "--half",
        action="store_true",
        help="Use FP16 precision"
    )

    parser.add_argument(
        "--one-to-many",
        action="store_true",
        help="Use one-to-many head (requires NMS, not recommended)"
    )

    args = parser.parse_args()

    # 모델 경로 확인 (경고만 표시하고 진행 - Ultralytics가 자동 다운로드)
    if not Path(args.model).exists():
        print(f"⚠️  Model file not found: {args.model}")
        print(f"📥 Ultralytics will automatically download the model...")
        print(f"\nAvailable YOLO26 models:")
        print("  yolo26n.pt  - Nano version (fastest, 5.3MB)")
        print("  yolo26s.pt  - Small version (9.5MB)")
        print("  yolo26m.pt  - Medium version (20.4MB)")
        print("  yolo26l.pt  - Large version (24.8MB)")
        print("  yolo26x.pt  - Extra Large version (most accurate, 55.7MB)")
        print()

    try:
        onnx_path = convert_to_onnx(
            model_path=args.model,
            imgsz=args.imgsz,
            simplify=not args.no_simplify,
            half=args.half,
            end2end=not args.one_to_many
        )

        print("\n📝 Next steps:")
        print(f"1. Copy {onnx_path} to humanoid_object_detection/yolo26/models/")
        print(f"2. Update config.model_path in your application")

        return 0

    except Exception as e:
        print(f"❌ Error during conversion: {e}")
        return 1


if __name__ == "__main__":
    exit(main())
