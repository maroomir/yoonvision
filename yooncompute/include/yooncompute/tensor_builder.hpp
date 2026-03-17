#ifndef YOONCOMPUTE_TENSOR_BUILDER_HPP
#define YOONCOMPUTE_TENSOR_BUILDER_HPP

#include <cstdint>
#include <vector>

#include "yooncompute/tensor.hpp"

namespace yoonvision {
namespace compute {

class TensorBuilder {
 public:
  TensorBuilder() = default;
  ~TensorBuilder() = default;

  TensorBuilder& WithDims(const std::vector<int64_t>& dims,
                          Tensor::DataType data_type = Tensor::DataType::FLOAT32);
  
  TensorBuilder& FromFloatBuffer(const std::vector<float>& data,
                                 const std::vector<int64_t>& dims,
                                 Tensor::DataType data_type = Tensor::DataType::FLOAT32);

  [[nodiscard]] Tensor Build() const;

  // [numpy 스타일]
  static Tensor Zeros(const std::vector<int64_t>& dims,
                      Tensor::DataType data_type = Tensor::DataType::FLOAT32);

  static Tensor Ones(const std::vector<int64_t>& dims,
                     Tensor::DataType data_type = Tensor::DataType::FLOAT32);

  static Tensor Full(const std::vector<int64_t>& dims,
                     float value,
                     Tensor::DataType data_type = Tensor::DataType::FLOAT32);

  static Tensor Arange(float start,
                       float stop,
                       float step = 1.0f,
                       Tensor::DataType data_type = Tensor::DataType::FLOAT32);

 private:
  enum class SourceType {
    kNone,
    kEmpty,        // dims + dtype 만 설정된 상태
    kFloatBuffer,  // 외부 float 버퍼로부터 생성
    kFull,         // scalar value 로 채우기
    kArange,       // arange 파라미터 기반
  };

  SourceType source_type_{SourceType::kNone};

  std::vector<int64_t> dims_{};
  Tensor::DataType data_type_{Tensor::DataType::FLOAT32};

  std::vector<float> float_buffer_{}; // [kFloatBuffer]
  
  float scalar_value_{0.0f};  // [kFull]

  float arange_start_{0.0f};  // [kArange]
  float arange_stop_{0.0f};   // [kArange]
  float arange_step_{1.0f};   // [kArange]
};

}  // namespace compute
}  // namespace yoonvision

#endif  // YOONCOMPUTE_TENSOR_BUILDER_HPP

