#include "yooncompute/tensor_processor.hpp"

#include <algorithm>
#include <numeric>

#include "yooncompute/tensor_utils.hpp"

namespace yoonvision {
namespace compute {

namespace {
Tensor TransposeImpl(const Tensor& src, const std::vector<int>& axes) {
  if (!tensor_utils::IsFloat32(src)) {
    return src;
  }

  const auto old_dims = src.GetDims();
  const int rank = static_cast<int>(old_dims.size());
  if (axes.size() != old_dims.size()) {
    return src;
  }

  std::vector<bool> used(rank, false);
  for (int ax : axes) {
    if (ax < 0 || ax >= rank || used[ax]) {
      return src;
    }
    used[ax] = true;
  }

  std::vector<int64_t> new_dims(rank);
  for (int i = 0; i < rank; ++i) {
    new_dims[i] = old_dims[axes[i]];
  }

  const auto src_data = src.As<float>();
  std::vector<float> dst_data(tensor_utils::NumElements(src));

  std::vector<size_t> old_stride(rank, 1);
  for (int i = rank - 2; i >= 0; --i) {
    old_stride[i] = old_stride[i + 1] *
                    static_cast<size_t>(old_dims[i + 1]);
  }

  std::vector<size_t> new_stride(rank, 1);
  for (int i = rank - 2; i >= 0; --i) {
    new_stride[i] = new_stride[i + 1] *
                    static_cast<size_t>(new_dims[i + 1]);
  }

  const size_t total = tensor_utils::NumElements(src);
  for (size_t idx = 0; idx < total; ++idx) {
    size_t tmp = idx;
    std::vector<size_t> new_coord(rank);
    for (int i = 0; i < rank; ++i) {
      new_coord[i] = tmp / new_stride[i];
      tmp %= new_stride[i];
    }

    std::vector<size_t> old_coord(rank);
    for (int i = 0; i < rank; ++i) {
      old_coord[axes[i]] = new_coord[i];
    }

    size_t old_idx = 0;
    for (int i = 0; i < rank; ++i) {
      old_idx += old_coord[i] * old_stride[i];
    }

    dst_data[idx] = src_data[old_idx];
  }

  return tensor_utils::MakeTensorWithDimsAndData(new_dims, src, dst_data);
}

enum class BinaryOp { kAdd, kMul };

Tensor BinaryOpImpl(const Tensor& a, const Tensor& b, BinaryOp op) {
  if (!tensor_utils::IsFloat32(a) || !tensor_utils::IsFloat32(b)) {
    return a;
  }

  const auto a_dims = a.GetDims();
  const auto b_dims = b.GetDims();
  const int a_rank = static_cast<int>(a_dims.size());
  const int b_rank = static_cast<int>(b_dims.size());

  const auto a_data = a.As<float>();
  const auto b_data = b.As<float>();

  // 1) 완전 동일 shape
  if (a_dims == b_dims) {
    std::vector<float> out(a_data.size());
    for (size_t i = 0; i < out.size(); ++i) {
      if (op == BinaryOp::kAdd) {
        out[i] = a_data[i] + b_data[i];
      } else {
        out[i] = a_data[i] * b_data[i];
      }
    }
    return tensor_utils::MakeTensorWithDimsAndData(a_dims, a, out);
  }

  // 2) b 가 스칼라 텐서(1 요소)인 경우
  if (b.TotalSize() == 1 && !b_data.empty()) {
    const float s = b_data[0];
    std::vector<float> out(a_data.size());
    for (size_t i = 0; i < out.size(); ++i) {
      if (op == BinaryOp::kAdd) {
        out[i] = a_data[i] + s;
      } else {
        out[i] = a_data[i] * s;
      }
    }
    return tensor_utils::MakeTensorWithDimsAndData(a_dims, a, out);
  }

  // 3) 채널 브로드캐스트: b_dims = {C}, a_dims = {C,H,W} 또는 {N,C,H,W}
  if (b_rank == 1 && a_rank >= 3) {
    const int64_t C = b_dims[0];
    if (a_rank == 3) {
      const int64_t C_a = a_dims[0];
      const int64_t H = a_dims[1];
      const int64_t W = a_dims[2];
      if (C_a != C) {
        return a;
      }

      std::vector<float> out(a_data.size());
      const size_t per_channel = static_cast<size_t>(H * W);
      for (int64_t c = 0; c < C; ++c) {
        const float w = b_data[static_cast<size_t>(c)];
        const size_t offset = static_cast<size_t>(c) * per_channel;
        for (size_t i = 0; i < per_channel; ++i) {
          const size_t idx = offset + i;
          if (op == BinaryOp::kAdd) {
            out[idx] = a_data[idx] + w;
          } else {
            out[idx] = a_data[idx] * w;
          }
        }
      }
      return tensor_utils::MakeTensorWithDimsAndData(a_dims, a, out);
    }

    if (a_rank == 4) {
      const int64_t N = a_dims[0];
      const int64_t C_a = a_dims[1];
      const int64_t H = a_dims[2];
      const int64_t W = a_dims[3];
      if (C_a != C) {
        return a;
      }

      std::vector<float> out(a_data.size());
      const size_t per_batch = static_cast<size_t>(C * H * W);
      const size_t per_channel = static_cast<size_t>(H * W);
      for (int64_t n = 0; n < N; ++n) {
        const size_t batch_offset = static_cast<size_t>(n) * per_batch;
        for (int64_t c = 0; c < C; ++c) {
          const float w = b_data[static_cast<size_t>(c)];
          const size_t ch_offset =
              batch_offset + static_cast<size_t>(c) * per_channel;
          for (size_t i = 0; i < per_channel; ++i) {
            const size_t idx = ch_offset + i;
            if (op == BinaryOp::kAdd) {
              out[idx] = a_data[idx] + w;
            } else {
              out[idx] = a_data[idx] * w;
            }
          }
        }
      }
      return tensor_utils::MakeTensorWithDimsAndData(a_dims, a, out);
    }
  }

  return a;
}

enum class ReductionOp { kSum, kMean, kMax };

Tensor ReduceImpl(const Tensor& src,
                  const std::vector<int>& axes,
                  bool keepdim,
                  ReductionOp op) {
  if (!tensor_utils::IsFloat32(src)) {
    return src;
  }

  auto dims = src.GetDims();
  const int rank = static_cast<int>(dims.size());
  if (rank == 0) {
    return src;
  }

  std::vector<int> norm_axes = axes;
  if (!tensor_utils::NormalizeAxes(dims, norm_axes)) {
    return src;
  }

  // 축 플래그: true 면 리덕션 대상
  std::vector<bool> reduce_flags(rank, false);
  for (int ax : norm_axes) {
    reduce_flags[ax] = true;
  }

  const std::vector<size_t> in_strides =
      tensor_utils::ComputeStrides(dims);

  std::vector<int64_t> out_dims;
  out_dims.reserve(rank);
  if (keepdim) {
    out_dims = dims;
    for (int ax : norm_axes) {
      out_dims[ax] = 1;
    }
  } else {
    for (int i = 0; i < rank; ++i) {
      if (!reduce_flags[i]) {
        out_dims.push_back(dims[i]);
      }
    }
    if (out_dims.empty()) {
      out_dims.push_back(1);
    }
  }

  const int out_rank = static_cast<int>(out_dims.size());
  const std::vector<size_t> out_strides =
      tensor_utils::ComputeStrides(out_dims);

  const auto src_data = src.As<float>();
  const size_t total = tensor_utils::NumElements(src);

  // 출력 버퍼 초기화
  const size_t out_total =
      std::accumulate(out_dims.begin(), out_dims.end(), static_cast<size_t>(1),
                      [](size_t acc, int64_t d) { return acc * static_cast<size_t>(d); });
  std::vector<float> out_data(out_total);

  if (op == ReductionOp::kMax) {
    // 매우 작은 값으로 초기화
    std::fill(out_data.begin(), out_data.end(),
              std::numeric_limits<float>::lowest());
  } else {
    std::fill(out_data.begin(), out_data.end(), 0.0f);
  }

  // 각 입력 원소를 대응되는 출력 인덱스로 매핑하며 누적
  for (size_t idx = 0; idx < total; ++idx) {
    std::vector<size_t> coord;
    tensor_utils::IndexToCoord(idx, in_strides, coord);

    // 좌표 → 출력 좌표
    std::vector<size_t> out_coord;
    out_coord.reserve(out_rank);
    if (keepdim) {
      out_coord.resize(rank);
      for (int i = 0; i < rank; ++i) {
        if (reduce_flags[i]) {
          out_coord[i] = 0;  // 리덕션 축은 0 위치
        } else {
          out_coord[i] = coord[i];
        }
      }
    } else {
      for (int i = 0; i < rank; ++i) {
        if (!reduce_flags[i]) {
          out_coord.push_back(coord[i]);
        }
      }
      if (out_coord.empty()) {
        out_coord.push_back(0);
      }
    }

    // 출력 좌표 → flat index
    const size_t out_idx =
        tensor_utils::CoordToIndex(out_coord, out_strides);

    const float v = src_data[idx];
    if (op == ReductionOp::kSum || op == ReductionOp::kMean) {
      out_data[out_idx] += v;
    } else if (op == ReductionOp::kMax) {
      out_data[out_idx] = std::max(out_data[out_idx], v);
    }
  }

  // mean 의 경우, 각 위치별로 리덕션된 요소 수로 나누기
  if (op == ReductionOp::kMean) {
    // 리덕션된 요소 수 = 리덕션 축 길이의 곱
    size_t reduce_count = 1;
    for (int ax = 0; ax < rank; ++ax) {
      if (reduce_flags[ax]) {
        reduce_count *= static_cast<size_t>(dims[ax]);
      }
    }
    if (reduce_count == 0) {
      reduce_count = 1;
    }
    const float inv = 1.0f / static_cast<float>(reduce_count);
    for (auto& v : out_data) {
      v *= inv;
    }
  }

  return tensor_utils::MakeTensorWithDimsAndData(out_dims, src, out_data);
}

Tensor ToCHWImpl(const Tensor& src) {
  const auto dims = src.GetDims();
  if (dims.size() == 3) {
    // HWC -> CHW
    const int64_t h = dims[0];
    const int64_t w = dims[1];
    const int64_t c = dims[2];
    return TransposeImpl(src, {2, 0, 1});  // HWC -> CHW
  }
  if (dims.size() == 4) {
    // NHWC -> NCHW
    return TransposeImpl(src, {0, 3, 1, 2});
  }
  return src;
}

Tensor ToHWCImpl(const Tensor& src) {
  const auto dims = src.GetDims();
  if (dims.size() == 3) {
    // CHW -> HWC
    return TransposeImpl(src, {1, 2, 0});
  }
  if (dims.size() == 4) {
    // NCHW -> NHWC
    return TransposeImpl(src, {0, 2, 3, 1});
  }
  return src;
}

Tensor ToNCHWImpl(const Tensor& src) {
  const auto dims = src.GetDims();
  if (dims.size() == 4) {
    // NHWC -> NCHW
    return TransposeImpl(src, {0, 3, 1, 2});
  }
  return src;
}

Tensor ToNHWCImpl(const Tensor& src) {
  const auto dims = src.GetDims();
  if (dims.size() == 4) {
    // NCHW -> NHWC
    return TransposeImpl(src, {0, 2, 3, 1});
  }
  return src;
}

}  // namespace

TensorProcessor TensorProcessor::FromTensor(const Tensor& tensor) {
  return TensorProcessor(tensor);
}

TensorProcessor::TensorProcessor(const Tensor& tensor) : tensor_(tensor) {}

TensorProcessor& TensorProcessor::Reshape(
    const std::vector<int64_t>& new_dims) {
  if (!tensor_utils::IsFloat32(tensor_)) {
    return *this;
  }

  const size_t old_size = tensor_utils::NumElements(tensor_);
  size_t new_size = 1;
  for (auto d : new_dims) {
    new_size *= static_cast<size_t>(d);
  }

  if (old_size != new_size) {
    return *this;
  }

  const auto data = tensor_.As<float>();
  tensor_ = tensor_utils::MakeTensorWithDimsAndData(new_dims, tensor_, data);
  return *this;
}

TensorProcessor& TensorProcessor::Squeeze(int dim) {
  auto dims = tensor_.GetDims();
  if (dims.empty()) {
    return *this;
  }

  if (dim >= 0) {
    if (dim >= static_cast<int>(dims.size()) || dims[dim] != 1) {
      return *this;
    }
    dims.erase(dims.begin() + dim);
  } else {  // [dim < 0]
    dims.erase(
        std::remove(dims.begin(), dims.end(), static_cast<int64_t>(1)),
        dims.end());
    if (dims.empty()) {
      dims.push_back(1);
    }
  }

  return Reshape(dims);
  return *this;
}

TensorProcessor& TensorProcessor::Unsqueeze(int dim) {
  auto dims = tensor_.GetDims();
  if (dim < 0) {
    dim = static_cast<int>(dims.size()) + dim + 1;
  }
  if (dim < 0 || dim > static_cast<int>(dims.size())) {
    return *this;
  }
  dims.insert(dims.begin() + dim, 1);
  return Reshape(dims);
  return *this;
}

TensorProcessor& TensorProcessor::Transpose(const std::vector<int>& axes) {
  tensor_ = TransposeImpl(tensor_, axes);
  return *this;
}

TensorProcessor& TensorProcessor::Add(float scalar) {
  if (!tensor_utils::IsFloat32(tensor_)) {
    return *this;
  }

  auto data = tensor_.As<float>();
  for (auto& v : data) {
    v += scalar;
  }
  tensor_ = tensor_utils::MakeTensorWithDimsAndData(tensor_.GetDims(),
                                                    tensor_, data);
  return *this;
}

TensorProcessor& TensorProcessor::Multiply(float scalar) {
  if (!tensor_utils::IsFloat32(tensor_)) {
    return *this;
  }

  auto data = tensor_.As<float>();
  for (auto& v : data) {
    v *= scalar;
  }
  tensor_ = tensor_utils::MakeTensorWithDimsAndData(tensor_.GetDims(),
                                                    tensor_, data);
  return *this;
}

TensorProcessor& TensorProcessor::Add(const Tensor& other) {
  tensor_ = BinaryOpImpl(tensor_, other, BinaryOp::kAdd);
  return *this;
}

TensorProcessor& TensorProcessor::Multiply(const Tensor& other) {
  tensor_ = BinaryOpImpl(tensor_, other, BinaryOp::kMul);
  return *this;
}

TensorProcessor& TensorProcessor::Normalize(float mean, float std) {
  norm_mean_ = mean;
  norm_std_ = (std == 0.0f) ? 1.0f : std;
  use_channel_wise_norm_ = false;
  norm_mean_vec_.clear();
  norm_std_vec_.clear();
  return *this;
}

TensorProcessor& TensorProcessor::Normalize(const std::vector<float>& mean,
                                            const std::vector<float>& std) {
  if (mean.empty() || std.empty() || mean.size() != std.size()) {
    return *this;
  }
  norm_mean_vec_ = mean;
  norm_std_vec_ = std;
  use_channel_wise_norm_ = true;
  return *this;
}

TensorProcessor& TensorProcessor::Clip(float min, float max) {
  if (!tensor_utils::IsFloat32(tensor_)) {
    return *this;
  }
  if (min > max) {
    return *this;
  }

  auto data = tensor_.As<float>();
  for (auto& v : data) {
    if (v < min) v = min;
    if (v > max) v = max;
  }

  tensor_ = tensor_utils::MakeTensorWithDimsAndData(tensor_.GetDims(),
                                                    tensor_, data);
  return *this;
}

TensorProcessor& TensorProcessor::MinMaxNormalize(float in_min,
                                                  float in_max,
                                                  float out_min,
                                                  float out_max) {
  if (!tensor_utils::IsFloat32(tensor_)) {
    return *this;
  }
  if (in_max == in_min) {
    return *this;
  }

  auto data = tensor_.As<float>();
  const float scale_in = 1.0f / (in_max - in_min);
  const float scale_out = (out_max - out_min);

  for (auto& v : data) {
    float t = (v - in_min) * scale_in;
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
    v = out_min + t * scale_out;
  }

  tensor_ = tensor_utils::MakeTensorWithDimsAndData(tensor_.GetDims(),
                                                    tensor_, data);
  return *this;
}

TensorProcessor& TensorProcessor::ToCHW() {
  tensor_ = ToCHWImpl(tensor_);
  return *this;
}

TensorProcessor& TensorProcessor::ToHWC() {
  tensor_ = ToHWCImpl(tensor_);
  return *this;
}

TensorProcessor& TensorProcessor::ToNCHW() {
  tensor_ = ToNCHWImpl(tensor_);
  return *this;
}

TensorProcessor& TensorProcessor::ToNHWC() {
  tensor_ = ToNHWCImpl(tensor_);
  return *this;
}

Tensor TensorProcessor::Concat(const std::vector<Tensor>& tensors, int axis) {
  if (tensors.empty()) {
    return Tensor();
  }
  const int rank = static_cast<int>(tensors[0].GetDims().size());
  if (rank == 0) {
    return Tensor();
  }
  if (axis < 0) {
    axis += rank;
  }
  if (axis < 0 || axis >= rank) {
    return Tensor();
  }

  const auto base_dims = tensors[0].GetDims();
  int64_t axis_sum = 0;
  for (const auto& t : tensors) {
    if (!tensor_utils::IsFloat32(t) ||
        t.GetDims().size() != base_dims.size()) {
      return Tensor();
    }
    const auto dims = t.GetDims();
    for (int i = 0; i < rank; ++i) {
      if (i == axis) continue;
      if (dims[i] != base_dims[i]) {
        return Tensor();
      }
    }
    axis_sum += dims[axis];
  }

  std::vector<int64_t> out_dims = base_dims;
  out_dims[axis] = axis_sum;

  Tensor out(out_dims, tensors[0].GetDataType());
  if (!out.Allocate()) {
    return Tensor();
  }

  auto out_data = out.As<float>();

  // 입력/출력 스트라이드 계산
  std::vector<size_t> out_strides =
      tensor_utils::ComputeStrides(out_dims);

  int64_t axis_offset = 0;
  for (const auto& t : tensors) {
    const auto dims = t.GetDims();
    const auto src_data = t.As<float>();

    std::vector<size_t> in_strides =
        tensor_utils::ComputeStrides(dims);

    const size_t total = tensor_utils::NumElements(t);
    for (size_t idx = 0; idx < total; ++idx) {
      std::vector<size_t> coord;
      tensor_utils::IndexToCoord(idx, in_strides, coord);

      coord[axis] += static_cast<size_t>(axis_offset);

      const size_t out_idx =
          tensor_utils::CoordToIndex(coord, out_strides);

      out_data[out_idx] = src_data[idx];
    }

    axis_offset += dims[axis];
  }

  out.SetFrom<float>(out_data);
  return out;
}

Tensor TensorProcessor::Stack(const std::vector<Tensor>& tensors, int axis) {
  if (tensors.empty()) {
    return Tensor();
  }
  const auto base_dims = tensors[0].GetDims();
  const int base_rank = static_cast<int>(base_dims.size());

  for (const auto& t : tensors) {
    if (!tensor_utils::IsFloat32(t) || t.GetDims() != base_dims) {
      return Tensor();
    }
  }

  int rank = base_rank + 1;
  if (axis < 0) {
    axis += rank;
  }
  if (axis < 0 || axis > rank) {
    return Tensor();
  }

  std::vector<int64_t> out_dims;
  out_dims.reserve(rank);
  for (int i = 0; i < rank; ++i) {
    if (i == axis) {
      out_dims.push_back(static_cast<int64_t>(tensors.size()));
    } else {
      int idx = i < axis ? i : i - 1;
      out_dims.push_back(base_dims[idx]);
    }
  }

  Tensor out(out_dims, tensors[0].GetDataType());
  if (!out.Allocate()) {
    return Tensor();
  }

  auto out_data = out.As<float>();
  const int out_rank = static_cast<int>(out_dims.size());
  std::vector<size_t> out_strides =
      tensor_utils::ComputeStrides(out_dims);

  const size_t per_tensor = tensor_utils::NumElements(tensors[0]);

  for (size_t t_idx = 0; t_idx < tensors.size(); ++t_idx) {
    const auto& t = tensors[t_idx];
    const auto src_data = t.As<float>();

    std::vector<size_t> in_strides =
        tensor_utils::ComputeStrides(base_dims);

    for (size_t idx = 0; idx < per_tensor; ++idx) {
      std::vector<size_t> in_coord;
      tensor_utils::IndexToCoord(idx, in_strides, in_coord);

      std::vector<size_t> out_coord(out_rank);
      for (int i = 0; i < out_rank; ++i) {
        if (i == axis) {
          out_coord[i] = t_idx;
        } else {
          int in_i = i < axis ? i : i - 1;
          out_coord[i] = in_coord[in_i];
        }
      }

      const size_t out_idx =
          tensor_utils::CoordToIndex(out_coord, out_strides);

      out_data[out_idx] = src_data[idx];
    }
  }

  out.SetFrom<float>(out_data);
  return out;
}

std::vector<Tensor> TensorProcessor::Split(
    const std::vector<int64_t>& sections, int axis) const {
  std::vector<Tensor> result;
  if (sections.empty()) {
    return result;
  }

  const auto dims = tensor_.GetDims();
  const int rank = static_cast<int>(dims.size());
  if (rank == 0) {
    return result;
  }
  if (axis < 0) {
    axis += rank;
  }
  if (axis < 0 || axis >= rank) {
    return result;
  }

  int64_t total_axis = dims[axis];
  int64_t sum_sections = 0;
  for (auto s : sections) {
    if (s <= 0) {
      return {};
    }
    sum_sections += s;
  }
  if (sum_sections != total_axis) {
    return {};
  }

  const auto src_data = tensor_.As<float>();

  std::vector<size_t> in_strides =
      tensor_utils::ComputeStrides(dims);

  int64_t axis_start = 0;
  for (auto sec : sections) {
    std::vector<int64_t> out_dims = dims;
    out_dims[axis] = sec;

    Tensor out(out_dims, tensor_.GetDataType());
    if (!out.Allocate()) {
      return {};
    }
    auto out_data = out.As<float>();

    std::vector<size_t> out_strides =
        tensor_utils::ComputeStrides(out_dims);

    const size_t out_total = tensor_utils::NumElements(out);
    for (size_t idx = 0; idx < out_total; ++idx) {
      std::vector<size_t> out_coord;
      tensor_utils::IndexToCoord(idx, out_strides, out_coord);

      std::vector<size_t> in_coord = out_coord;
      in_coord[axis] += static_cast<size_t>(axis_start);

      const size_t in_idx =
          tensor_utils::CoordToIndex(in_coord, in_strides);

      out_data[idx] = src_data[in_idx];
    }

    out.SetFrom<float>(out_data);
    result.push_back(out);
    axis_start += sec;
  }

  return result;
}

TensorProcessor& TensorProcessor::Crop(int64_t top,
                                       int64_t left,
                                       int64_t height,
                                       int64_t width) {
  if (!tensor_utils::IsFloat32(tensor_)) {
    return *this;
  }

  const auto dims = tensor_.GetDims();
  const size_t rank = dims.size();
  if (rank != 3 && rank != 4) {
    return *this;
  }

  int64_t C, H, W, N = 1;
  bool has_batch = (rank == 4);
  if (has_batch) {
    N = dims[0];
    C = dims[1];
    H = dims[2];
    W = dims[3];
  } else {
    C = dims[0];
    H = dims[1];
    W = dims[2];
  }

  if (top < 0 || left < 0 || height <= 0 || width <= 0) {
    return *this;
  }
  if (top + height > H || left + width > W) {
    return *this;
  }

  std::vector<int64_t> out_dims;
  if (has_batch) {
    out_dims = {N, C, height, width};
  } else {
    out_dims = {C, height, width};
  }

  const auto src_data = tensor_.As<float>();
  Tensor out(out_dims, tensor_.GetDataType());
  if (!out.Allocate()) {
    return *this;
  }
  auto out_data = out.As<float>();

  if (has_batch) {
    const size_t src_stride_w = 1;
    const size_t src_stride_h = static_cast<size_t>(W);
    const size_t src_stride_c = static_cast<size_t>(H * W);
    const size_t src_stride_n = static_cast<size_t>(C) * src_stride_c;

    const size_t dst_stride_w = 1;
    const size_t dst_stride_h = static_cast<size_t>(width);
    const size_t dst_stride_c = static_cast<size_t>(height * width);
    const size_t dst_stride_n = static_cast<size_t>(C) * dst_stride_c;

    for (int64_t n = 0; n < N; ++n) {
      for (int64_t c = 0; c < C; ++c) {
        for (int64_t h = 0; h < height; ++h) {
          for (int64_t w = 0; w < width; ++w) {
            const size_t src_index =
                static_cast<size_t>(n) * src_stride_n +
                static_cast<size_t>(c) * src_stride_c +
                static_cast<size_t>(top + h) * src_stride_h +
                static_cast<size_t>(left + w) * src_stride_w;
            const size_t dst_index =
                static_cast<size_t>(n) * dst_stride_n +
                static_cast<size_t>(c) * dst_stride_c +
                static_cast<size_t>(h) * dst_stride_h +
                static_cast<size_t>(w) * dst_stride_w;
            out_data[dst_index] = src_data[src_index];
          }
        }
      }
    }
  } else {
    const size_t src_stride_w = 1;
    const size_t src_stride_h = static_cast<size_t>(W);
    const size_t src_stride_c = static_cast<size_t>(H * W);

    const size_t dst_stride_w = 1;
    const size_t dst_stride_h = static_cast<size_t>(width);
    const size_t dst_stride_c = static_cast<size_t>(height * width);

    for (int64_t c = 0; c < C; ++c) {
      for (int64_t h = 0; h < height; ++h) {
        for (int64_t w = 0; w < width; ++w) {
          const size_t src_index =
              static_cast<size_t>(c) * src_stride_c +
              static_cast<size_t>(top + h) * src_stride_h +
              static_cast<size_t>(left + w) * src_stride_w;
          const size_t dst_index =
              static_cast<size_t>(c) * dst_stride_c +
              static_cast<size_t>(h) * dst_stride_h +
              static_cast<size_t>(w) * dst_stride_w;
          out_data[dst_index] = src_data[src_index];
        }
      }
    }
  }

  out.SetFrom<float>(out_data);
  tensor_ = out;
  return *this;
}

TensorProcessor& TensorProcessor::Pad(int64_t top,
                                      int64_t bottom,
                                      int64_t left,
                                      int64_t right,
                                      float value) {
  if (!tensor_utils::IsFloat32(tensor_)) {
    return *this;
  }

  const auto dims = tensor_.GetDims();
  const size_t rank = dims.size();
  if (rank != 3 && rank != 4) {
    return *this;
  }

  if (top < 0 || bottom < 0 || left < 0 || right < 0) {
    return *this;
  }

  int64_t C, H, W, N = 1;
  bool has_batch = (rank == 4);
  if (has_batch) {
    N = dims[0];
    C = dims[1];
    H = dims[2];
    W = dims[3];
  } else {
    C = dims[0];
    H = dims[1];
    W = dims[2];
  }

  const int64_t new_h = H + top + bottom;
  const int64_t new_w = W + left + right;

  std::vector<int64_t> out_dims;
  if (has_batch) {
    out_dims = {N, C, new_h, new_w};
  } else {
    out_dims = {C, new_h, new_w};
  }

  const auto src_data = tensor_.As<float>();
  Tensor out(out_dims, tensor_.GetDataType());
  if (!out.Allocate()) {
    return *this;
  }
  auto out_data = out.As<float>();
  std::fill(out_data.begin(), out_data.end(), value);

  if (has_batch) {
    const size_t src_stride_w = 1;
    const size_t src_stride_h = static_cast<size_t>(W);
    const size_t src_stride_c = static_cast<size_t>(H * W);
    const size_t src_stride_n = static_cast<size_t>(C) * src_stride_c;

    const size_t dst_stride_w = 1;
    const size_t dst_stride_h = static_cast<size_t>(new_w);
    const size_t dst_stride_c = static_cast<size_t>(new_h * new_w);
    const size_t dst_stride_n = static_cast<size_t>(C) * dst_stride_c;

    for (int64_t n = 0; n < N; ++n) {
      for (int64_t c = 0; c < C; ++c) {
        for (int64_t h = 0; h < H; ++h) {
          for (int64_t w = 0; w < W; ++w) {
            const size_t src_index =
                static_cast<size_t>(n) * src_stride_n +
                static_cast<size_t>(c) * src_stride_c +
                static_cast<size_t>(h) * src_stride_h +
                static_cast<size_t>(w) * src_stride_w;
            const size_t dst_index =
                static_cast<size_t>(n) * dst_stride_n +
                static_cast<size_t>(c) * dst_stride_c +
                static_cast<size_t>(h + top) * dst_stride_h +
                static_cast<size_t>(w + left) * dst_stride_w;
            out_data[dst_index] = src_data[src_index];
          }
        }
      }
    }
  } else {
    const size_t src_stride_w = 1;
    const size_t src_stride_h = static_cast<size_t>(W);
    const size_t src_stride_c = static_cast<size_t>(H * W);

    const size_t dst_stride_w = 1;
    const size_t dst_stride_h = static_cast<size_t>(new_w);
    const size_t dst_stride_c = static_cast<size_t>(new_h * new_w);

    for (int64_t c = 0; c < C; ++c) {
      for (int64_t h = 0; h < H; ++h) {
        for (int64_t w = 0; w < W; ++w) {
          const size_t src_index =
              static_cast<size_t>(c) * src_stride_c +
              static_cast<size_t>(h) * src_stride_h +
              static_cast<size_t>(w) * src_stride_w;
          const size_t dst_index =
              static_cast<size_t>(c) * dst_stride_c +
              static_cast<size_t>(h + top) * dst_stride_h +
              static_cast<size_t>(w + left) * dst_stride_w;
          out_data[dst_index] = src_data[src_index];
        }
      }
    }
  }

  out.SetFrom<float>(out_data);
  tensor_ = out;
  return *this;
}

Tensor TensorProcessor::Argmax(int axis) const {
  if (!tensor_utils::IsFloat32(tensor_)) {
    return Tensor();
  }

  const auto dims = tensor_.GetDims();
  int axis_norm = axis;
  if (!tensor_utils::NormalizeAxis(dims, axis_norm)) {
    return Tensor();
  }

  const auto src_data = tensor_.As<float>();

  std::vector<int64_t> out_dims = dims;
  out_dims[axis_norm] = 1;

  Tensor out(out_dims, Tensor::DataType::INT32);
  if (!out.Allocate()) {
    return Tensor();
  }

  auto out_raw = out.GetRaw_data();
  std::vector<int32_t> out_data(out.TotalSize(), 0);

  const int rank = static_cast<int>(dims.size());
  const std::vector<size_t> in_strides =
      tensor_utils::ComputeStrides(dims);
  const std::vector<size_t> out_strides =
      tensor_utils::ComputeStrides(out_dims);

  const size_t axis_len = static_cast<size_t>(dims[axis_norm]);
  const size_t total = tensor_utils::NumElements(tensor_);

  std::fill(out_data.begin(), out_data.end(), 0);
  for (size_t out_idx = 0; out_idx < out_data.size(); ++out_idx) {
    std::vector<size_t> coord;
    tensor_utils::IndexToCoord(out_idx, out_strides, coord);

    float best_val = std::numeric_limits<float>::lowest();
    size_t best_axis = 0;

    for (size_t k = 0; k < axis_len; ++k) {
      std::vector<size_t> in_coord = coord;
      in_coord[axis_norm] = k;

      const size_t in_idx =
          tensor_utils::CoordToIndex(in_coord, in_strides);

      const float v = src_data[in_idx];
      if (v > best_val) {
        best_val = v;
        best_axis = k;
      }
    }

    out_data[out_idx] = static_cast<int32_t>(best_axis);
  }

  out.SetFrom<int32_t>(out_data);
  (void)out_raw;
  return out;
}

Tensor TensorProcessor::Softmax(int axis) const {
  if (!tensor_utils::IsFloat32(tensor_)) {
    return Tensor();
  }

  const auto dims = tensor_.GetDims();
  const int rank = static_cast<int>(dims.size());
  if (rank == 0) {
    return Tensor();
  }
  if (axis < 0) {
    axis += rank;
  }
  if (axis < 0 || axis >= rank) {
    return Tensor();
  }

  const auto src_data = tensor_.As<float>();

  Tensor out(dims, tensor_.GetDataType());
  if (!out.Allocate()) {
    return Tensor();
  }
  auto out_data = out.As<float>();

  std::vector<size_t> strides =
      tensor_utils::ComputeStrides(dims);

  const size_t axis_len = static_cast<size_t>(dims[axis]);
  const size_t inner = strides[axis];
  const size_t outer =
      tensor_utils::NumElements(tensor_) / (axis_len * inner);

  for (size_t o = 0; o < outer; ++o) {
    for (size_t i = 0; i < inner; ++i) {
      float max_val = std::numeric_limits<float>::lowest();
      for (size_t k = 0; k < axis_len; ++k) {
        size_t idx = o * axis_len * inner + k * inner + i;
        max_val = std::max(max_val, src_data[idx]);
      }

      float sum = 0.0f;
      for (size_t k = 0; k < axis_len; ++k) {
        size_t idx = o * axis_len * inner + k * inner + i;
        float e = std::exp(src_data[idx] - max_val);
        out_data[idx] = e;
        sum += e;
      }

      if (sum == 0.0f) {
        continue;
      }
      const float inv = 1.0f / sum;
      for (size_t k = 0; k < axis_len; ++k) {
        size_t idx = o * axis_len * inner + k * inner + i;
        out_data[idx] *= inv;
      }
    }
  }

  out.SetFrom<float>(out_data);
  return out;
}

Tensor TensorProcessor::Sigmoid() const {
  if (!tensor_utils::IsFloat32(tensor_)) {
    return Tensor();
  }

  const auto dims = tensor_.GetDims();
  const auto src_data = tensor_.As<float>();

  Tensor out(dims, tensor_.GetDataType());
  if (!out.Allocate()) {
    return Tensor();
  }
  auto out_data = out.As<float>();

  const size_t total = tensor_utils::NumElements(tensor_);
  for (size_t i = 0; i < total; ++i) {
    const float x = src_data[i];
    out_data[i] = 1.0f / (1.0f + std::exp(-x));
  }

  out.SetFrom<float>(out_data);
  return out;
}

Tensor TensorProcessor::Process() const {
  return tensor_;
}

std::vector<float> TensorProcessor::ToFlatVector() const {
  if (!tensor_utils::IsFloat32(tensor_)) {
    return {};
  }

  std::vector<float> data = tensor_.As<float>();
  if (data.empty()) {
    return data;
  }

  if (!use_channel_wise_norm_) {
    const float mean = norm_mean_;
    const float std = (norm_std_ == 0.0f) ? 1.0f : norm_std_;
    for (auto& v : data) {
      v = (v - mean) / std;
    }
    return data;
  }

  const auto dims = tensor_.GetDims();
  if (dims.empty()) {
    return data;
  }

  const size_t c = static_cast<size_t>(dims[0]);
  if (c != norm_mean_vec_.size() || c != norm_std_vec_.size()) {
    return data;
  }

  const size_t total = data.size();
  if (c == 0 || total % c != 0) {
    return data;
  }

  const size_t per_channel = total / c;
  for (size_t ch = 0; ch < c; ++ch) {
    float mean = norm_mean_vec_[ch];
    float std = norm_std_vec_[ch];
    if (std == 0.0f) {
      std = 1.0f;
    }
    const size_t offset = ch * per_channel;
    for (size_t i = 0; i < per_channel; ++i) {
      data[offset + i] =
          (data[offset + i] - mean) / std;
    }
  }

  return data;
}

}  // namespace compute
}  // namespace yoonvision

