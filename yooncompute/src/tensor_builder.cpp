#include "yooncompute/tensor_builder.hpp"

#include <cmath>

namespace yoonvision {
namespace compute {

TensorBuilder& TensorBuilder::WithDims(const std::vector<int64_t>& dims,
                                       Tensor::DataType data_type) {
  dims_ = dims;
  data_type_ = data_type;
  source_type_ = SourceType::kEmpty;
  float_buffer_.clear();
  return *this;
}

TensorBuilder& TensorBuilder::FromFloatBuffer(const std::vector<float>& data,
                                              const std::vector<int64_t>& dims,
                                              Tensor::DataType data_type) {
  dims_ = dims;
  data_type_ = data_type;
  source_type_ = SourceType::kFloatBuffer;
  float_buffer_ = data;
  return *this;
}

Tensor TensorBuilder::Build() const {
  Tensor tensor(dims_, data_type_);

  switch (source_type_) {
    case SourceType::kEmpty: {
      if (!tensor.Allocate()) {
        return Tensor();
      }
      return tensor;
    }

    case SourceType::kFloatBuffer: {
      if (!tensor.Allocate()) {
        return Tensor();
      }
      tensor.SetFrom<float>(float_buffer_);
      return tensor;
    }

    case SourceType::kFull: {
      if (!tensor.Allocate()) {
        return Tensor();
      }
      std::vector<float> data(tensor.TotalSize(), scalar_value_);
      tensor.SetFrom<float>(data);
      return tensor;
    }

    case SourceType::kArange: {
      if (!tensor.Allocate()) {
        return Tensor();
      }
      if (arange_step_ == 0.0f) {
        return Tensor();
      }

      const float length =
          (arange_stop_ - arange_start_) / arange_step_;
      const auto count =
          static_cast<size_t>(std::max(0.0f, std::floor(length)));

      std::vector<float> data;
      data.reserve(count);
      float value = arange_start_;
      for (size_t i = 0; i < count; ++i) {
        data.push_back(value);
        value += arange_step_;
      }

      tensor.SetFrom<float>(data);
      return tensor;
    }

    case SourceType::kNone:
    default:
      return Tensor();
  }
}

Tensor TensorBuilder::Zeros(const std::vector<int64_t>& dims,
                            Tensor::DataType data_type) {
  Tensor tensor(dims, data_type);
  if (!tensor.Allocate()) {
    return Tensor();
  }
  tensor.Zero();
  return tensor;
}

Tensor TensorBuilder::Ones(const std::vector<int64_t>& dims,
                           Tensor::DataType data_type) {
  Tensor tensor(dims, data_type);
  if (!tensor.Allocate()) {
    return Tensor();
  }

  const size_t size = tensor.TotalSize();
  std::vector<float> data(size, 1.0f);
  tensor.SetFrom<float>(data);
  return tensor;
}

Tensor TensorBuilder::Full(const std::vector<int64_t>& dims,
                           float value,
                           Tensor::DataType data_type) {
  TensorBuilder builder;
  builder.dims_ = dims;
  builder.data_type_ = data_type;
  builder.source_type_ = SourceType::kFull;
  builder.scalar_value_ = value;
  return builder.Build();
}

Tensor TensorBuilder::Arange(float start,
                             float stop,
                             float step,
                             Tensor::DataType data_type) {
  TensorBuilder builder;
  builder.dims_ = {0};  // 실제 길이는 Build 시점에서 결정
  builder.data_type_ = data_type;
  builder.source_type_ = SourceType::kArange;
  builder.arange_start_ = start;
  builder.arange_stop_ = stop;
  builder.arange_step_ = step;

  if (step != 0.0f) {
    const float length = (stop - start) / step;
    const auto count =
        static_cast<int64_t>(std::max(0.0f, std::floor(length)));
    builder.dims_ = {count};
  }

  return builder.Build();
}

}  // namespace compute
}  // namespace yoonvision

