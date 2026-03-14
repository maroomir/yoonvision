#ifndef YOONCOMPUTE_TENSOR_HPP
#define YOONCOMPUTE_TENSOR_HPP

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace yoonvision {
namespace compute {

class Tensor {
 public:
  enum class DataType { FLOAT32, FLOAT16, INT32, INT8, UINT8 };

  Tensor();
  Tensor(const std::vector<int64_t>& dims, DataType data_type);
  Tensor(const std::vector<int64_t>& dims, DataType data_type, const std::vector<uint8_t>& data);
  Tensor(const Tensor& other);
  Tensor(Tensor&& other) noexcept;
  Tensor& operator=(const Tensor& other);
  Tensor& operator=(Tensor&& other) noexcept;

  ~Tensor() = default;

  inline std::vector<int64_t> GetDims() const { return dims_; }
  inline DataType GetDataType() const { return data_type_; }
  inline std::string GetName() const { return name_; }
  void SetName(const std::string& name) { name_ = name; }

  inline size_t TotalSize() const {
    size_t size = 1;
    for (auto dim : dims_) size *= dim;
    return size;
  }

  inline size_t ByteSize() const { return raw_data_.size(); }
  inline const std::vector<uint8_t>& GetRaw_data() const { return raw_data_; }

  size_t DataTypeSize() const;

  bool Allocate();

  bool Zero();

  bool IsValid() const;

  std::string Info() const;

  template <typename T>
  std::vector<T> As() const;

  template <typename T>
  bool SetFrom(const std::vector<T>& data);

 private:
  std::vector<int64_t> dims_;
  DataType data_type_;
  std::vector<uint8_t> raw_data_;
  std::string name_;
};

}  // namespace compute
}  // namespace yoonvision

#endif  // YOONCOMPUTE_TENSOR_HPP
