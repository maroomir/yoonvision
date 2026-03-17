#ifndef YOONCOMPUTE_TENSOR_UTILS_HPP
#define YOONCOMPUTE_TENSOR_UTILS_HPP

#include <cstdint>
#include <numeric>
#include <vector>

#include "yooncompute/tensor.hpp"

namespace yoonvision {
namespace compute {
namespace tensor_utils {

inline bool IsFloat32(const Tensor& tensor) {
  return tensor.GetDataType() == Tensor::DataType::FLOAT32;
}

inline size_t NumElements(const Tensor& tensor) {
  return tensor.TotalSize();
}

inline Tensor MakeTensorWithDimsAndData(const std::vector<int64_t>& dims,
                                        const Tensor& src_like,
                                        const std::vector<float>& data) {
  Tensor t(dims, src_like.GetDataType());
  if (!t.Allocate()) {
    return Tensor();
  }
  t.SetFrom<float>(data);
  return t;
}

inline std::vector<size_t> ComputeStrides(const std::vector<int64_t>& dims) {
  const int rank = static_cast<int>(dims.size());
  std::vector<size_t> strides(rank, 1);
  for (int i = rank - 2; i >= 0; --i) {
    strides[i] = strides[i + 1] * static_cast<size_t>(dims[i + 1]);
  }
  return strides;
}

inline void IndexToCoord(size_t idx,
                         const std::vector<size_t>& strides,
                         std::vector<size_t>& coord) {
  const int rank = static_cast<int>(strides.size());
  coord.resize(rank);
  size_t tmp = idx;
  for (int i = 0; i < rank; ++i) {
    coord[i] = tmp / strides[i];
    tmp %= strides[i];
  }
}

inline size_t CoordToIndex(const std::vector<size_t>& coord,
                           const std::vector<size_t>& strides) {
  const int rank = static_cast<int>(strides.size());
  size_t idx = 0;
  for (int i = 0; i < rank; ++i) {
    idx += coord[i] * strides[i];
  }
  return idx;
}

inline bool NormalizeAxis(const std::vector<int64_t>& dims, int& axis) {
  const int rank = static_cast<int>(dims.size());
  if (rank == 0) return false;
  if (axis < 0) {
    axis += rank;
  }
  return axis >= 0 && axis < rank;
}

inline bool NormalizeAxes(const std::vector<int64_t>& dims,
                          std::vector<int>& axes) {
  const int rank = static_cast<int>(dims.size());
  if (rank == 0) return false;

  if (axes.empty()) {
    axes.resize(rank);
    std::iota(axes.begin(), axes.end(), 0);
  } else {
    for (auto& ax : axes) {
      if (ax < 0) {
        ax += rank;
      }
      if (ax < 0 || ax >= rank) {
        return false;
      }
    }
  }

  std::sort(axes.begin(), axes.end());
  axes.erase(std::unique(axes.begin(), axes.end()), axes.end());
  return true;
}

}  // namespace tensor_utils
}  // namespace compute
}  // namespace yoonvision

#endif  // YOONCOMPUTE_TENSOR_UTILS_HPP

