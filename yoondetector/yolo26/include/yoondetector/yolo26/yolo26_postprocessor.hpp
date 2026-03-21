//
// YOLO26 extension - YOLO-style output decoding (skeleton; extend per ONNX export)
//

#ifndef YOONVISION_YOONDETECTOR_YOLO26_POSTPROCESSOR_HPP
#define YOONVISION_YOONDETECTOR_YOLO26_POSTPROCESSOR_HPP

#include "yoondetector/detector.hpp"
#include "yoondetector/postprocessor.hpp"

namespace yoonvision {
namespace detector {

class Yolo26Postprocessor : public Postprocessor {
 public:
  DetectionResult Run(const compute::Tensor& output,
                      const DetectorParameter& parameter,
                      const Image& original) const override;
};

}  // namespace detector
}  // namespace yoonvision

#endif  // YOONVISION_YOONDETECTOR_YOLO26_POSTPROCESSOR_HPP
