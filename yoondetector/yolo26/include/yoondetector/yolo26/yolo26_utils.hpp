//
// YOLO26 shared helpers (preprocess / postprocess); header-only.
//

#ifndef YOONVISION_YOONDETECTOR_YOLO26_UTILS_HPP
#define YOONVISION_YOONDETECTOR_YOLO26_UTILS_HPP

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

#include "yoondetector/detector.hpp"
#include "yoondetector/yolo26/coco_class_names.hpp"

namespace yoonvision {
namespace detector {
namespace yolo26 {

struct ParsedDetectionRow {
  float x1 = 0.0f;
  float y1 = 0.0f;
  float x2 = 0.0f;
  float y2 = 0.0f;
  float score = 0.0f;
  int class_id = -1;
};

struct BoxCxCyWh {
  float cx = 0.0f;
  float cy = 0.0f;
  float w = 0.0f;
  float h = 0.0f;
};

inline bool IsClose(float lhs, float rhs, float eps = 1e-6f) {
  return std::fabs(lhs - rhs) < eps;
}

// Ultralytics-style default: mean 0, std 1 per channel → use ImageProcessor 0/255.
inline bool HasCustomNormalization(const DetectorParameter& parameter) {
  const bool mean_is_default = IsClose(parameter.mean[0], 0.0f) &&
                               IsClose(parameter.mean[1], 0.0f) &&
                               IsClose(parameter.mean[2], 0.0f);
  const bool std_is_default = IsClose(parameter.std[0], 1.0f) &&
                              IsClose(parameter.std[1], 1.0f) &&
                              IsClose(parameter.std[2], 1.0f);
  return !(mean_is_default && std_is_default);
}

// Matches ImageProcessor::LetterboxNearest padding (input-space pixels).
inline void ComputeLetterboxParams(int src_w,
                                   int src_h,
                                   int input_w,
                                   int input_h,
                                   float& scale,
                                   float& pad_w,
                                   float& pad_h) {
  const float scale_w =
      static_cast<float>(input_w) / static_cast<float>(src_w);
  const float scale_h =
      static_cast<float>(input_h) / static_cast<float>(src_h);
  scale = std::min(scale_w, scale_h);

  const int resized_w = static_cast<int>(static_cast<float>(src_w) * scale);
  const int resized_h = static_cast<int>(static_cast<float>(src_h) * scale);
  pad_w = static_cast<float>((input_w - resized_w) / 2);
  pad_h = static_cast<float>((input_h - resized_h) / 2);
}

inline BoxCxCyWh ClampToImageBounds(const BoxCxCyWh& box, int image_w,
                                    int image_h) {
  const float max_x = static_cast<float>(std::max(0, image_w - 1));
  const float max_y = static_cast<float>(std::max(0, image_h - 1));

  float x1 = box.cx - (box.w * 0.5f);
  float y1 = box.cy - (box.h * 0.5f);
  float x2 = box.cx + (box.w * 0.5f);
  float y2 = box.cy + (box.h * 0.5f);

  x1 = std::max(0.0f, std::min(x1, max_x));
  y1 = std::max(0.0f, std::min(y1, max_y));
  x2 = std::max(0.0f, std::min(x2, max_x));
  y2 = std::max(0.0f, std::min(y2, max_y));

  BoxCxCyWh clamped;
  clamped.cx = (x1 + x2) * 0.5f;
  clamped.cy = (y1 + y2) * 0.5f;
  clamped.w = std::max(0.0f, x2 - x1);
  clamped.h = std::max(0.0f, y2 - y1);
  return clamped;
}

inline Detection ToDetection(const BoxCxCyWh& box, float score, int class_id) {
  Detection det;
  det.bbox.cx = box.cx;
  det.bbox.cy = box.cy;
  det.bbox.w = box.w;
  det.bbox.h = box.h;
  det.score = score;
  det.class_id = class_id;
  det.class_name = ClassNameForYoloDetection(class_id);
  return det;
}

inline ParsedDetectionRow ParseNx6Row(const std::vector<float>& data, int index) {
  const std::size_t base = static_cast<std::size_t>(index) * 6;
  ParsedDetectionRow row;
  row.x1 = data[base + 0];
  row.y1 = data[base + 1];
  row.x2 = data[base + 2];
  row.y2 = data[base + 3];
  row.score = data[base + 4];
  row.class_id = static_cast<int>(data[base + 5]);
  return row;
}

inline ParsedDetectionRow Parse6xNRow(const std::vector<float>& data, int index,
                                      int num_detections) {
  const std::size_t stride = static_cast<std::size_t>(num_detections);
  const std::size_t i = static_cast<std::size_t>(index);
  ParsedDetectionRow row;
  row.x1 = data[0 * stride + i];
  row.y1 = data[1 * stride + i];
  row.x2 = data[2 * stride + i];
  row.y2 = data[3 * stride + i];
  row.score = data[4 * stride + i];
  row.class_id = static_cast<int>(data[5 * stride + i]);
  return row;
}

inline BoxCxCyWh DecodeToOriginal(const ParsedDetectionRow& row,
                                  const DetectorParameter& parameter,
                                  int orig_w, int orig_h) {
  BoxCxCyWh box;
  box.cx = (row.x1 + row.x2) * 0.5f;
  box.cy = (row.y1 + row.y2) * 0.5f;
  box.w = row.x2 - row.x1;
  box.h = row.y2 - row.y1;

  if (parameter.letterbox) {
    float scale = 1.0f;
    float pad_w = 0.0f;
    float pad_h = 0.0f;
    ComputeLetterboxParams(orig_w, orig_h, parameter.input_width,
                           parameter.input_height, scale, pad_w, pad_h);

    if (!IsClose(scale, 0.0f)) {
      box.cx = (box.cx - pad_w) / scale;
      box.cy = (box.cy - pad_h) / scale;
      box.w /= scale;
      box.h /= scale;
    }
  } else {
    const float scale_x =
        static_cast<float>(orig_w) / static_cast<float>(parameter.input_width);
    const float scale_y = static_cast<float>(orig_h) /
                          static_cast<float>(parameter.input_height);
    box.cx *= scale_x;
    box.cy *= scale_y;
    box.w *= scale_x;
    box.h *= scale_y;
  }

  return ClampToImageBounds(box, orig_w, orig_h);
}

// End-to-end Ultralytics ONNX: [1, N, 6] or [1, 6, N].
struct End2EndLayoutInfo {
  bool valid = false;
  bool nx6_layout = true;  // true: [1,N,6], false: [1,6,N]
  int num_detections = 0;
};

inline End2EndLayoutInfo InferEnd2EndLayout(const std::vector<int64_t>& dims) {
  End2EndLayoutInfo info;
  if (dims.size() != 3 || dims[0] != 1) {
    return info;
  }
  if (dims[2] == 6) {
    info.valid = true;
    info.nx6_layout = true;
    info.num_detections = static_cast<int>(dims[1]);
    return info;
  }
  if (dims[1] == 6) {
    info.valid = true;
    info.nx6_layout = false;
    info.num_detections = static_cast<int>(dims[2]);
    return info;
  }
  return info;
}

}  // namespace yolo26
}  // namespace detector
}  // namespace yoonvision

#endif  // YOONVISION_YOONDETECTOR_YOLO26_UTILS_HPP
