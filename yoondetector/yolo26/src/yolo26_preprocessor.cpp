//
// YOLO26 extension - default preprocessor implementation
//

#include "yoondetector/yolo26/yolo26_preprocessor.hpp"

#include <vector>

#include "yoondetector/yolo26/yolo26_utils.hpp"
#include "yooncompute/tensor_builder.hpp"
#include "yoonvision/image_processor.hpp"

namespace yoonvision {
namespace detector {

compute::Tensor Yolo26Preprocessor::Run(const Image& image,
                                         const DetectorParameter& parameter) const {
  ImageProcessor proc = ImageProcessor::FromImage(image);

  switch (parameter.color_space) {
    case ColorSpace::kRGB:
      proc.ToRgb();
      break;
    case ColorSpace::kBGR:
      proc.ToBgr();
      break;
    default:
      proc.ToRgb();
      break;
  }

  if (parameter.letterbox) {
    proc.Letterbox(parameter.input_width, parameter.input_height, 0);
  } else {
    proc.Resize(parameter.input_width, parameter.input_height);
  }

  // Keep ImageProcessor default normalization (0/255) for Ultralytics-style YOLO
  // unless the caller explicitly supplies custom channel-wise mean/std.
  if (parameter.normalize && yolo26::HasCustomNormalization(parameter)) {
    proc.Normalize({parameter.mean[0], parameter.mean[1], parameter.mean[2]},
                   {parameter.std[0], parameter.std[1], parameter.std[2]});
  }

  std::vector<float> buffer = proc.ToFloatCHW();

  const auto width = static_cast<int64_t>(parameter.input_width);
  const auto height = static_cast<int64_t>(parameter.input_height);
  std::vector<int64_t> dims = {1, 3, height, width};

  compute::TensorBuilder builder;
  builder.FromFloatBuffer(buffer, dims, compute::Tensor::DataType::FLOAT32);
  return builder.Build();
}

}  // namespace detector
}  // namespace yoonvision
