//
// YOLO26 extension - default image-to-tensor preprocessing for YOLO-style models
//

#ifndef YOONVISION_YOONDETECTOR_YOLO26_PREPROCESSOR_HPP
#define YOONVISION_YOONDETECTOR_YOLO26_PREPROCESSOR_HPP

#include "yoondetector/detector.hpp"
#include "yoondetector/preprocessor.hpp"

namespace yoonvision {
namespace detector {

class Yolo26Preprocessor : public Preprocessor {
 public:
  compute::Tensor Run(const Image& image,
                      const DetectorParameter& parameter) const override;
};

}  // namespace detector
}  // namespace yoonvision

#endif  // YOONVISION_YOONDETECTOR_YOLO26_PREPROCESSOR_HPP
