//
// YOLO26 extension - YOLO-like postprocessor (skeleton)
//

#include "yoondetector/yolo26/yolo26_postprocessor.hpp"

#include <vector>

#include "log.hpp"
#include "yoondetector/yolo26/yolo26_utils.hpp"

namespace yoonvision {
namespace detector {

DetectionResult Yolo26Postprocessor::Run(const compute::Tensor& output,
                                       const DetectorParameter& parameter,
                                       const Image& original) const {
  DetectionResult result;

  if (!output.IsValid()) {
    LOG_WARN("Yolo26Postprocessor::Run: output tensor is invalid");
    return result;
  }

  if (output.GetDataType() != compute::Tensor::DataType::FLOAT32) {
    LOG_WARN(
        "Yolo26Postprocessor::Run: unsupported output dtype (expected FLOAT32)");
    return result;
  }

  const yolo26::End2EndLayoutInfo layout =
      yolo26::InferEnd2EndLayout(output.GetDims());
  if (!layout.valid) {
    LOG_WARN("Yolo26Postprocessor::Run: unexpected output dims");
    return result;
  }

  const int orig_w = static_cast<int>(original.GetWidth());
  const int orig_h = static_cast<int>(original.GetHeight());
  if (orig_w <= 0 || orig_h <= 0 || parameter.input_width <= 0 ||
      parameter.input_height <= 0) {
    LOG_WARN("Yolo26Postprocessor::Run: invalid image or input dimensions");
    return result;
  }

  const int num_detections = layout.num_detections;
  if (num_detections <= 0) {
    return result;
  }

  const std::vector<float> raw = output.As<float>();
  const std::size_t required = static_cast<std::size_t>(num_detections) * 6;
  if (raw.size() < required) {
    LOG_WARN("Yolo26Postprocessor::Run: output data is smaller than expected");
    return result;
  }

  for (int i = 0; i < num_detections; ++i) {
    const yolo26::ParsedDetectionRow row =
        layout.nx6_layout ? yolo26::ParseNx6Row(raw, i)
                          : yolo26::Parse6xNRow(raw, i, num_detections);

    if (row.score < parameter.confidence_threshold) {
      continue;
    }

    const yolo26::BoxCxCyWh box =
        yolo26::DecodeToOriginal(row, parameter, orig_w, orig_h);
    if (box.w <= 0.0f || box.h <= 0.0f) {
      continue;
    }

    result.detections.push_back(yolo26::ToDetection(box, row.score, row.class_id));
  }

  if (parameter.max_detections > 0 &&
      static_cast<int>(result.detections.size()) > parameter.max_detections) {
    result.detections.resize(static_cast<std::size_t>(parameter.max_detections));
  }

  return result;
}

}  // namespace detector
}  // namespace yoonvision
