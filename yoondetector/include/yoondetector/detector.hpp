//
// yoonvision yoondetector - detector interface, parameters, and state
// (aligned with yooncamera: CameraParameter / CameraState)
//

#ifndef YOONVISION_YOONDETECTOR_DETECTOR_HPP
#define YOONVISION_YOONDETECTOR_DETECTOR_HPP

#include <array>
#include <cstddef>
#include <memory>
#include <string>
#include <vector>

#include "yooncompute/processor.hpp"
#include "yoonvision/image.hpp"

namespace yoonvision {
namespace detector {

enum class ColorSpace {
  kRGB = 0,
  kBGR = 1,
};

struct DetectorParameter {
  std::string model_path;
  std::string model_type;  // e.g. "yolo26"

  float confidence_threshold = 0.5f;
  float nms_threshold = 0.45f;
  int max_detections = 300;

  int input_width = 640;
  int input_height = 640;
  bool letterbox = true;
  bool normalize = true;
  std::array<float, 3> mean{{0.0f, 0.0f, 0.0f}};
  std::array<float, 3> std{{1.0f, 1.0f, 1.0f}};
  ColorSpace color_space = ColorSpace::kRGB;

  compute::Processor::Provider provider = compute::Processor::Provider::CPU;
  int device_id = 0;

  struct Condition {
    bool model_file_exists = false;
    bool gpu_available = false;
    bool cuda_supported = false;
    bool model_compatible = false;
    bool input_dimensions_valid = false;
  } condition;
};

struct DetectorState {
  enum class Status { kUnknown = 0, kSucceeded, kFailed };

  Status load_model = Status::kUnknown;
  Status initialize_processor = Status::kUnknown;
  Status preprocess = Status::kUnknown;
  Status inference = Status::kUnknown;
  Status postprocess = Status::kUnknown;

  struct Monitor {
    std::size_t success_inference_count = 0;
    std::size_t failed_inference_count = 0;
    std::size_t total_frames_processed = 0;
    double avg_inference_time_ms = 0.0;
  } monitor;
};

struct Detection {
  struct BoundingBox {
    float cx = 0.0f;
    float cy = 0.0f;
    float w = 0.0f;
    float h = 0.0f;
  } bbox;

  float score = 0.0f;
  int class_id = -1;
  std::string class_name;
};

struct DetectionResult {
  std::vector<Detection> detections;
  double inference_time_ms = 0.0;
};

class Detector {
 public:
  virtual ~Detector() = default;

  virtual bool Initialize(const DetectorParameter& parameter) = 0;

  virtual DetectionResult Detect(const Image& image) = 0;

  virtual std::vector<DetectionResult> DetectBatch(
      const std::vector<Image>& images) = 0;

  virtual bool IsInitialized() const = 0;

  virtual DetectorParameter GetParameter() const = 0;

  virtual DetectorState GetDetectorState() const = 0;

  virtual std::string GetModelName() const = 0;

  virtual std::string GetModelVersion() const = 0;
};

using DetectorPtr = std::unique_ptr<Detector>;

}  // namespace detector
}  // namespace yoonvision

#endif  // YOONVISION_YOONDETECTOR_DETECTOR_HPP
