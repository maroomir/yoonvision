//
// yoonvision yoondetector - preprocessing interface
//

#ifndef YOONVISION_YOONDETECTOR_PREPROCESSOR_HPP
#define YOONVISION_YOONDETECTOR_PREPROCESSOR_HPP

#include <memory>

#include "yooncompute/tensor.hpp"
#include "yoonvision/image.hpp"
#include "yoondetector/detector.hpp"

namespace yoonvision {
namespace detector {

class Preprocessor {
 public:
  virtual ~Preprocessor() = default;

  // Convert input image into model-ready tensor using config.
  virtual compute::Tensor Run(const Image& image,
                              const DetectorParameter& parameter) const = 0;
};

using PreprocessorPtr = std::unique_ptr<Preprocessor>;

}  // namespace detector
}  // namespace yoonvision

#endif  // YOONVISION_YOONDETECTOR_PREPROCESSOR_HPP

