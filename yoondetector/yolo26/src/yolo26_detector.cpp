//
// yoonvision yoondetector - YOLO26 detector implementation
//

#include "yoondetector/yolo26/yolo26_detector.hpp"

#include <chrono>
#include <utility>

#include "log.hpp"
#include "yoondetector/detector_factory.hpp"

namespace yoonvision {
namespace detector {

namespace {

bool RegisterYolo26() {
  DetectorFactory::RegisterDetector(
      "yolo26", []() { return std::unique_ptr<Detector>(new Yolo26Detector()); });
  return true;
}

const bool kYolo26Registered = RegisterYolo26();

}  // namespace

Yolo26Detector::Yolo26Detector()
    : initialized_(false),
      parameter_(),
      state_(),
      processor_(),
      preprocessor_(new Yolo26Preprocessor()),
      postprocessor_(new Yolo26Postprocessor()) {}

bool Yolo26Detector::Register() {
  return kYolo26Registered;
}

bool Yolo26Detector::Initialize(const DetectorParameter& parameter) {
  parameter_ = parameter;

  if (parameter_.model_path.empty()) {
    LOG_ERROR("Yolo26Detector::Initialize: model_path is empty");
    initialized_ = false;
    return false;
  }

  if (!processor_.Initialize(parameter_.provider, parameter_.device_id)) {
    LOG_ERROR("Yolo26Detector::Initialize: Processor initialization failed");
    initialized_ = false;
    return false;
  }

  const bool low_processor_mode = true;
  if (!processor_.LoadModel(parameter_.model_path, low_processor_mode)) {
    LOG_ERROR("Yolo26Detector::Initialize: failed to load model: %s",
              parameter_.model_path.c_str());
    initialized_ = false;
    return false;
  }

  initialized_ = true;
  return true;
}

DetectionResult Yolo26Detector::Detect(const Image& image) {
  DetectionResult result;

  if (!initialized_) {
    LOG_ERROR("Yolo26Detector::Detect called before Initialize");
    return result;
  }

  if (!preprocessor_) {
    LOG_ERROR("Yolo26Detector::Detect: preprocessor is null");
    return result;
  }
  if (!postprocessor_) {
    LOG_ERROR("Yolo26Detector::Detect: postprocessor is null");
    return result;
  }

  const auto start_time = std::chrono::steady_clock::now();

  compute::Tensor input_tensor = preprocessor_->Run(image, parameter_);
  if (!input_tensor.IsValid()) {
    LOG_ERROR("Yolo26Detector::Detect: invalid input tensor from preprocessor");
    state_.monitor.failed_inference_count++;
    return result;
  }

  compute::Tensor output_tensor = processor_.Run(input_tensor);
  if (!output_tensor.IsValid()) {
    LOG_ERROR("Yolo26Detector::Detect: inference failed");
    state_.monitor.failed_inference_count++;
    return result;
  }

  result = postprocessor_->Run(output_tensor, parameter_, image);

  const auto end_time = std::chrono::steady_clock::now();
  const auto diff_ms =
      std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time)
          .count();
  result.inference_time_ms = static_cast<double>(diff_ms);

  state_.monitor.total_frames_processed++;
  if (!result.detections.empty()) {
    state_.monitor.success_inference_count++;
  } else {
    state_.monitor.failed_inference_count++;
  }

  const std::size_t count = state_.monitor.success_inference_count +
                            state_.monitor.failed_inference_count;
  if (count > 0) {
    state_.monitor.avg_inference_time_ms =
        (state_.monitor.avg_inference_time_ms * static_cast<double>(count - 1) +
         result.inference_time_ms) /
        static_cast<double>(count);
  }

  return result;
}

std::vector<DetectionResult> Yolo26Detector::DetectBatch(
    const std::vector<Image>& images) {
  std::vector<DetectionResult> results;
  results.reserve(images.size());

  for (const auto& img : images) {
    results.push_back(Detect(img));
  }

  return results;
}

}  // namespace detector
}  // namespace yoonvision

