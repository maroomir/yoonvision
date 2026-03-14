#include "yooncompute/tensor.hpp"

#include <cstring>
#include <sstream>

namespace yoonvision {
namespace compute {

Tensor::Tensor() : data_type_(DataType::FLOAT32) {}

Tensor::Tensor(const std::vector<int64_t>& dims, DataType data_type)
    : dims_(dims), data_type_(data_type) {
  Allocate();
}

Tensor::Tensor(const std::vector<int64_t>& dims, DataType data_type,
               const std::vector<uint8_t>& data)
    : dims_(dims), data_type_(data_type), raw_data_(data) {}

// COPY
Tensor::Tensor(const Tensor& other)
    : dims_(other.dims_),
      data_type_(other.data_type_),
      raw_data_(other.raw_data_),
      name_(other.name_) {}

// MOVE
Tensor::Tensor(Tensor&& other) noexcept
    : dims_(std::move(other.dims_)),
      data_type_(other.data_type_),
      raw_data_(std::move(other.raw_data_)),
      name_(std::move(other.name_)) {}

// 복사 대입
Tensor& Tensor::operator=(const Tensor& other) {
  if (this != &other) {
    dims_ = other.dims_;
    data_type_ = other.data_type_;
    raw_data_ = other.raw_data_;
    name_ = other.name_;
  }
  return *this;
}

// 이동 대입
Tensor& Tensor::operator=(Tensor&& other) noexcept {
  if (this != &other) {
    dims_ = std::move(other.dims_);
    data_type_ = other.data_type_;
    raw_data_ = std::move(other.raw_data_);
    name_ = std::move(other.name_);
  }
  return *this;
}

size_t Tensor::DataTypeSize() const {
  switch (data_type_) {
    case DataType::FLOAT32:
      return sizeof(float);
    case DataType::FLOAT16:
      return 2;
    case DataType::INT32:
      return sizeof(int32_t);
    case DataType::INT8:
      return sizeof(int8_t);
    case DataType::UINT8:
      return sizeof(uint8_t);
    default:
      return 0;
  }
}

bool Tensor::Allocate() {
  size_t total_bytes = TotalSize() * DataTypeSize();
  if (total_bytes == 0) {
    return false;
  }

  try {
    raw_data_.resize(total_bytes);
    return true;
  } catch (const std::bad_alloc&) {
    return false;
  }
}

bool Tensor::Zero() {
  if (raw_data_.empty()) {
    return false;
  }

  std::memset(raw_data_.data(), 0, raw_data_.size());
  return true;
}

bool Tensor::IsValid() const {
  if (dims_.empty()) {
    return false;
  }
  for (auto dim : dims_) {
    if (dim <= 0) {
      return false;
    }
  }
  if (DataTypeSize() == 0) {
    return false;
  }
  size_t expected_bytes = TotalSize() * DataTypeSize();
  if (raw_data_.size() != expected_bytes) {
    return false;
  }
  return true;
}

std::string Tensor::Info() const {
  std::ostringstream oss;
  oss << "Tensor{";
  oss << "name: " << (name_.empty() ? "\"\"" : "\"" + name_ + "\"") << ", ";
  oss << "dims: [";
  for (size_t i = 0; i < dims_.size(); ++i) {
    if (i > 0) oss << ", ";
    oss << dims_[i];
  }
  oss << "], ";
  oss << "dtype: ";
  switch (data_type_) {
    case DataType::FLOAT32:
      oss << "FLOAT32";
      break;
    case DataType::FLOAT16:
      oss << "FLOAT16";
      break;
    case DataType::INT32:
      oss << "INT32";
      break;
    case DataType::INT8:
      oss << "INT8";
      break;
    case DataType::UINT8:
      oss << "UINT8";
      break;
  }
  oss << ", ";
  oss << "total_elements: " << TotalSize() << ", ";
  oss << "byte_size: " << ByteSize();
  oss << "}";
  return oss.str();
}

template <typename T>
std::vector<T> Tensor::As() const {
  size_t num_elements = TotalSize();
  const T* data_ptr = reinterpret_cast<const T*>(raw_data_.data());
  return std::vector<T>(data_ptr, data_ptr + num_elements);
}

template <typename T>
bool Tensor::SetFrom(const std::vector<T>& data) {
  if (data.empty()) {
    return false;
  }

  size_t expected_size = TotalSize();
  if (data.size() != expected_size) {
    return false;
  }

  size_t byte_count = data.size() * sizeof(T);
  const uint8_t* byte_ptr = reinterpret_cast<const uint8_t*>(data.data());
  raw_data_.assign(byte_ptr, byte_ptr + byte_count);

  return true;
}

template std::vector<float> Tensor::As<float>() const;
template std::vector<int32_t> Tensor::As<int32_t>() const;
template std::vector<uint8_t> Tensor::As<uint8_t>() const;
template std::vector<int8_t> Tensor::As<int8_t>() const;

template bool Tensor::SetFrom<float>(const std::vector<float>& data);
template bool Tensor::SetFrom<int32_t>(const std::vector<int32_t>& data);
template bool Tensor::SetFrom<uint8_t>(const std::vector<uint8_t>& data);
template bool Tensor::SetFrom<int8_t>(const std::vector<int8_t>& data);

}  // namespace compute
}  // namespace yoonvision
