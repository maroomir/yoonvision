//
// yoonvision yoondetector - postprocessing interface
//

#ifndef YOONVISION_YOONDETECTOR_POSTPROCESSOR_HPP
#define YOONVISION_YOONDETECTOR_POSTPROCESSOR_HPP

#include <memory>

#include "yooncompute/tensor.hpp"
#include "yoonvision/image.hpp"
#include "yoondetector/detector.hpp"

namespace yoonvision {
namespace detector {

class Postprocessor {
 public:
  virtual ~Postprocessor() = default;

  virtual DetectionResult Run(const compute::Tensor& output,
                              const DetectorParameter& parameter,
                              const Image& original) const = 0;
};

using PostprocessorPtr = std::unique_ptr<Postprocessor>;

}  // namespace detector
}  // namespace yoonvision

#endif  // YOONVISION_YOONDETECTOR_POSTPROCESSOR_HPP

