#ifndef YOONCOMPUTE_PROCESSOR_HPP
#define YOONCOMPUTE_PROCESSOR_HPP

#include <onnxruntime_cxx_api.h>

#include <memory>
#include <string>
#include <vector>

#include "yooncompute/tensor.hpp"

namespace yoonvision {
namespace compute {

class Processor {
 public:
  enum class Provider { CPU, CUDA, COREML };

  Processor();
  ~Processor() = default;

  bool Initialize(Provider ep = Provider::CPU, int device_id = 0);

  void Cleanup();

  bool LoadModel(const std::string& model_path, const bool& low_processor_mode = true);

  Tensor Run(const Tensor& input);

  std::string GetModelPath() const { return model_path_; }

  bool IsModelLoaded() const { return (ort_session_ != nullptr); }

  Provider GetExecutionProvider() const { return ep_; }

 private:
  bool ValidateInput(const Tensor& input);

  bool SetupCudaProvider(int device_id);
  bool SetupCoreMLProvider(uint32_t flags);

  std::unique_ptr<Ort::Env> ort_env_;
  std::unique_ptr<Ort::Session> ort_session_;
  std::unique_ptr<Ort::MemoryInfo> ort_memory_info_;
  std::unique_ptr<Ort::SessionOptions> ort_session_options_;
  std::vector<Ort::AllocatedStringPtr> ort_input_names_;
  std::vector<Ort::AllocatedStringPtr> ort_output_names_;

  Provider ep_;
  int device_id_;
  std::string model_path_;
};

}  // namespace compute
}  // namespace yoonvision

#endif  // YOONCOMPUTE_PROCESSOR_HPP
