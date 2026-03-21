//
// yoonvision yoondetector - detector factory implementation
//

#include "yoondetector/detector_factory.hpp"

#include <stdexcept>

namespace yoonvision {
namespace detector {

std::unordered_map<std::string, DetectorCreator>& DetectorFactory::GetCreators() {
  static std::unordered_map<std::string, DetectorCreator> creators;
  return creators;
}

void DetectorFactory::RegisterDetector(const std::string& model_type,
                                       DetectorCreator creator) {
  GetCreators()[model_type] = std::move(creator);
}

std::unique_ptr<Detector> DetectorFactory::CreateDetector(
    const std::string& model_type) {
  auto& creators = GetCreators();
  auto it = creators.find(model_type);
  if (it != creators.end()) {
    return it->second();
  }

  throw std::runtime_error("Unsupported model type: " + model_type);
}

bool DetectorFactory::IsModelTypeSupported(const std::string& model_type) {
  return GetCreators().find(model_type) != GetCreators().end();
}

std::vector<std::string> DetectorFactory::GetSupportedModels() {
  auto& creators = GetCreators();
  std::vector<std::string> models;
  models.reserve(creators.size());

  for (const auto& kv : creators) {
    models.push_back(kv.first);
  }

  return models;
}

}  // namespace detector
}  // namespace yoonvision

