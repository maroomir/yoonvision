//
// yoonvision yoondetector - detector factory
//

#ifndef YOONVISION_YOONDETECTOR_DETECTOR_FACTORY_HPP
#define YOONVISION_YOONDETECTOR_DETECTOR_FACTORY_HPP

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "yoondetector/detector.hpp"

namespace yoonvision {
namespace detector {

using DetectorCreator = std::function<std::unique_ptr<Detector>()>;

class DetectorFactory {
 public:
  static void RegisterDetector(const std::string& model_type,
                               DetectorCreator creator);

  static std::unique_ptr<Detector> CreateDetector(
      const std::string& model_type);

  static bool IsModelTypeSupported(const std::string& model_type);

  static std::vector<std::string> GetSupportedModels();

 private:
  static std::unordered_map<std::string, DetectorCreator>& GetCreators();
};

}  // namespace detector
}  // namespace yoonvision

#endif  // YOONVISION_YOONDETECTOR_DETECTOR_FACTORY_HPP

