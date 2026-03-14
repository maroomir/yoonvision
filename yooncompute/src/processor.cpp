#include "yooncompute/processor.hpp"

#include <iostream>

#include "log.hpp"

// CoreML Provider Header (ONNX Runtime 에 CoreML 기능이 포함되어 빌드된 경우에만 활성화)
#ifdef USE_COREML
#include <coreml_provider_factory.h>
#endif

namespace yoonvision {
namespace compute {

namespace {
constexpr int kRuntimeLogLevel = 2;  // [0: Debug, 1:Info, 2: Warning, 3: Error, 4: Fatal]
constexpr int kLimitedSessionThreadNum = 1;
}  // namespace

Processor::Processor()
    : ort_env_(nullptr),
      ort_session_(nullptr),
      ort_memory_info_(nullptr),
      ort_session_options_(nullptr),
      ep_(Provider::CPU),
      device_id_(0) {}

bool Processor::Initialize(Provider ep, int device_id) {
  LOG_INFO("Initializing ONNXPipeline...");

  ep_ = ep;
  device_id_ = device_id;

  ort_env_ = std::make_unique<Ort::Env>(ORT_LOGGING_LEVEL_WARNING, "ONNXPipeline");

  LOG_INFO("ONNXPipeline initialized successfully with %s provider (device: %d)",
           ep_ == Provider::CUDA ? "CUDA" : (ep_ == Provider::COREML ? "COREML" : "CPU"),
           device_id_);
  return true;
}

void Processor::Cleanup() {
  LOG_INFO("Cleaning up ONNXPipeline");

  ort_session_.reset();
  ort_memory_info_.reset();
  ort_session_options_.reset();

  ort_input_names_.clear();
  ort_output_names_.clear();

  model_path_.clear();

  LOG_INFO("ONNXPipeline cleaned up");
}

bool Processor::LoadModel(const std::string& model_path, const bool& low_processor_mode) {
  LOG_INFO("Loading model: %s", model_path.c_str());

  try {
    ort_session_.reset();
    ort_memory_info_.reset();

    ort_session_options_ = std::make_unique<Ort::SessionOptions>();

    ort_session_options_->SetLogSeverityLevel(kRuntimeLogLevel);

    if (ep_ == Provider::CUDA) {
      if (!SetupCudaProvider(device_id_)) {
        LOG_WARN("Failed to setup CUDA provider, falling back to CPU");
      } else {
        LOG_INFO("CUDA provider configured successfully");
      }
    } else if (ep_ == Provider::COREML) {
      if (!SetupCoreMLProvider(0)) {
        LOG_WARN("Failed to setup CoreML provider, falling back to CPU");
      } else {
        LOG_INFO("CoreML provider configured successfully");
      }
    }

    if (low_processor_mode) {
      LOG_WARN("Active low processor mode. Runtime session optimizing");
      ort_session_options_->SetIntraOpNumThreads(kLimitedSessionThreadNum);
      ort_session_options_->SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_BASIC);
    } else {
      ort_session_options_->SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
    }

    ort_session_ =
        std::make_unique<Ort::Session>(*ort_env_, model_path.c_str(), *ort_session_options_);

    Ort::MemoryInfo memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
    ort_memory_info_ = std::make_unique<Ort::MemoryInfo>(std::move(memory_info));

    Ort::AllocatorWithDefaultOptions allocator;

    Ort::AllocatedStringPtr allocated_input_name =
        ort_session_->GetInputNameAllocated(0, allocator);
    ort_input_names_.push_back(std::move(allocated_input_name));
    if (!ort_input_names_.empty()) {
      LOG_INFO("Model input name: %s", ort_input_names_[0].get());
    }

    Ort::AllocatedStringPtr allocated_output_name =
        ort_session_->GetOutputNameAllocated(0, allocator);
    ort_output_names_.push_back(std::move(allocated_output_name));
    if (!ort_output_names_.empty()) {
      LOG_INFO("Model output name: %s", ort_output_names_[0].get());
    }

    model_path_ = model_path;
    LOG_INFO("Model loaded successfully with %s execution provider",
             ep_ == Provider::CUDA ? "CUDA" : (ep_ == Provider::COREML ? "COREML" : "CPU"));
    return true;
  } catch (const Ort::Exception& e) {
    LOG_ERROR("ONNX Runtime exception: %s", e.what());
    return false;
  }
}

Tensor Processor::Run(const Tensor& input) {
  if (ort_session_ == nullptr) {
    LOG_ERROR("ONNX Runtime session is not initialized");
    return Tensor();
  }

  if (!ValidateInput(input)) {
    return Tensor();
  }

  if (ort_input_names_.empty() || ort_output_names_.empty()) {
    LOG_ERROR("ONNX Runtime input/output names are not set");
    return Tensor();
  }

  try {
    auto input_data = input.As<float>();
    std::vector<int64_t> input_dims = input.GetDims();

    auto input_tensor =
        Ort::Value::CreateTensor<float>(*ort_memory_info_, input_data.data(), input.TotalSize(),
                                        input_dims.data(), input_dims.size());

    const char* input_names[] = {ort_input_names_[0].get()};
    const char* output_names[] = {ort_output_names_[0].get()};

    auto output_tensors =
        ort_session_->Run(Ort::RunOptions{nullptr}, input_names, &input_tensor, 1, output_names, 1);

    auto& output_tensor = output_tensors[0];
    auto output_dims = output_tensor.GetTensorTypeAndShapeInfo().GetShape();

    float* output_data_ptr = output_tensor.GetTensorMutableData<float>();

    Tensor output(output_dims, Tensor::DataType::FLOAT32);
    if (!output.IsValid()) {
      LOG_ERROR("Failed to create output tensor");
      return Tensor();
    }

    size_t output_size = 1;
    for (auto dim : output_dims) {
      output_size *= dim;
    }

    std::vector<float> output_data(output_data_ptr, output_data_ptr + output_size);
    output.SetFrom(output_data);

    return output;
  } catch (const Ort::Exception& e) {
    LOG_ERROR("ONNX Runtime exception during inference: %s", e.what());
    return Tensor();
  }
}

bool Processor::ValidateInput(const Tensor& input) {
  if (!input.IsValid()) {
    LOG_ERROR("Invalid input tensor");
    return false;
  }

  if (input.GetDataType() != Tensor::DataType::FLOAT32) {
    LOG_ERROR("Only FLOAT32 data type is supported");
    return false;
  }

  if (input.TotalSize() == 0) {
    LOG_ERROR("Input tensor is empty");
    return false;
  }

  return true;
}

bool Processor::SetupCudaProvider(int device_id) {
#ifdef ENABLE_CUDA
  try {
    OrtCUDAProviderOptions cuda_options;
    cuda_options.device_id = device_id;
    ort_session_options_->AppendExecutionProvider_CUDA(cuda_options);
    LOG_INFO("CUDA execution provider setup successful on device %d", device_id);
    return true;
  } catch (const Ort::Exception& e) {
    LOG_ERROR("Failed to setup CUDA execution provider: %s", e.what());
    return false;
  }
#else
  LOG_WARN("CUDA provider is not enabled in this build. Define ENABLE_CUDA to enable it.");
  return false;
#endif
}

bool Processor::SetupCoreMLProvider(uint32_t flags) {
#ifdef ENABLE_COREML
  try {
    OrtStatus* status =
        Ort::GetApi().SessionOptionsAppendExecutionProvider_CoreML(*ort_session_options_, flags);
    if (status != nullptr) {
      LOG_ERROR("Failed to setup CoreML execution provider.");
      Ort::GetApi().ReleaseStatus(status);
      return false;
    }
    LOG_INFO("CoreML execution provider setup successful with flags: %u", flags);
    return true;
  } catch (const Ort::Exception& e) {
    LOG_ERROR("Exception while setting up CoreML: %s", e.what());
    return false;
  }
#else
  LOG_WARN(
      "CoreML provider is not enabled in this build. Define ENABLE_COREML to enable it. (You must "
      "link against an ONNX Runtime build with CoreML support.)");
  return false;
#endif
}

}  // namespace compute
}  // namespace yoonvision
