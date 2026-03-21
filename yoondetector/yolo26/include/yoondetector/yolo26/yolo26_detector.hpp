//
// yoonvision yoondetector - YOLO26 detector (ONNXRuntime via yooncompute)
//

#ifndef YOONVISION_YOONDETECTOR_YOLO26_DETECTOR_HPP
#define YOONVISION_YOONDETECTOR_YOLO26_DETECTOR_HPP

#include <memory>
#include <string>
#include <vector>

#include "yooncompute/processor.hpp"
#include "yoonvision/image.hpp"
#include "yoondetector/detector.hpp"
#include "yoondetector/yolo26/yolo26_postprocessor.hpp"
#include "yoondetector/yolo26/yolo26_preprocessor.hpp"

namespace yoonvision {
namespace detector {

class Yolo26Detector : public Detector {
 public:
  Yolo26Detector();
  ~Yolo26Detector() override = default;

  bool Initialize(const DetectorParameter& parameter) override;

  DetectionResult Detect(const Image& image) override;

  std::vector<DetectionResult> DetectBatch(
      const std::vector<Image>& images) override;

  bool IsInitialized() const override { return initialized_; }

  DetectorParameter GetParameter() const override { return parameter_; }

  DetectorState GetDetectorState() const override { return state_; }

  std::string GetModelName() const override { return "YOLO26"; }

  std::string GetModelVersion() const override { return "unknown"; }

  // Registration helper for DetectorFactory.
  static bool Register();

 private:
  bool initialized_;
  DetectorParameter parameter_;
  DetectorState state_;

  compute::Processor processor_;
  PreprocessorPtr preprocessor_;
  PostprocessorPtr postprocessor_;
};

}  // namespace detector
}  // namespace yoonvision

#endif  // YOONVISION_YOONDETECTOR_YOLO26_DETECTOR_HPP

