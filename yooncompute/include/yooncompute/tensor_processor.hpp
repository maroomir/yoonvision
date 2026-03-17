#ifndef YOONCOMPUTE_TENSOR_PROCESSOR_HPP
#define YOONCOMPUTE_TENSOR_PROCESSOR_HPP

#include <cstdint>
#include <vector>

#include "yooncompute/tensor.hpp"

namespace yoonvision {
namespace compute {

class TensorProcessor {
 public:
  static TensorProcessor FromTensor(const Tensor& tensor);

  TensorProcessor& Reshape(const std::vector<int64_t>& new_dims);
  TensorProcessor& Squeeze(int dim = -1);
  TensorProcessor& Unsqueeze(int dim);

  TensorProcessor& Transpose(const std::vector<int>& axes);

  TensorProcessor& Add(float scalar);
  TensorProcessor& Multiply(float scalar);

  TensorProcessor& Add(const Tensor& other);
  TensorProcessor& Multiply(const Tensor& other);

  TensorProcessor& Normalize(float mean = 0.0f, float std = 1.0f);
  TensorProcessor& Normalize(const std::vector<float>& mean,
                             const std::vector<float>& std);

  TensorProcessor& Clip(float min, float max);
  TensorProcessor& MinMaxNormalize(float in_min,
                                   float in_max,
                                   float out_min = 0.0f,
                                   float out_max = 1.0f);

  TensorProcessor& ToCHW();
  TensorProcessor& ToHWC();
  TensorProcessor& ToNCHW();
  TensorProcessor& ToNHWC();

  static Tensor Concat(const std::vector<Tensor>& tensors, int axis);
  static Tensor Stack(const std::vector<Tensor>& tensors, int axis);
  std::vector<Tensor> Split(const std::vector<int64_t>& sections,
                            int axis) const;

  TensorProcessor& Crop(int64_t top, int64_t left,
                        int64_t height, int64_t width);
  TensorProcessor& Pad(int64_t top, int64_t bottom,
                       int64_t left, int64_t right,
                       float value = 0.0f);

  Tensor Argmax(int axis) const;
  Tensor Softmax(int axis) const;
  Tensor Sigmoid() const;

  [[nodiscard]] Tensor Process() const;
  std::vector<float> ToFlatVector() const;

 private:
  explicit TensorProcessor(const Tensor& tensor);

  Tensor tensor_;

  float norm_mean_{0.0f};
  float norm_std_{1.0f};
  bool use_channel_wise_norm_{false};
  std::vector<float> norm_mean_vec_{};
  std::vector<float> norm_std_vec_{};
};

}  // namespace compute
}  // namespace yoonvision

#endif  // YOONCOMPUTE_TENSOR_PROCESSOR_HPP

