#include "yoonvision_viewer/vision_application.hpp"

#include <atomic>
#include <chrono>
#include <filesystem>
#include <thread>

#include "yoonvision_viewer/frame_publisher_subscriber.hpp"
#include "log.hpp"
#include "yooncamera/camera_factory.hpp"
#include "yoondetector/detector_factory.hpp"

#ifndef YOONVISION_YOLO26_MODEL_PATH
#define YOONVISION_YOLO26_MODEL_PATH \
  "yoondetector/yolo26/models/yolo26n.onnx"
#endif

extern std::atomic<bool> g_should_shutdown;

namespace yoonvision::viewer {

namespace {
constexpr int kMainLoopSleepMs = 10;
constexpr char kRawStreamId[] = "webcam";
constexpr char kOverlayStreamId[] = "webcam_yolo";
constexpr char kCameraType[] = "avfcam";
constexpr uint16_t kCameraTimeoutMs = 2000;
constexpr std::uint32_t kInferenceIntervalFrames = 3;
}  // namespace

VisionApplication::VisionApplication()
    : http_server_(std::make_shared<http::HTTPServer>()), initialized_(false) {}

VisionApplication::~VisionApplication() {
  Shutdown();
}

bool VisionApplication::Initialize(int port,
                                    const std::string& resources_path) {
  LOG_INFO("Initializing VisionApplication...");

  auto param = std::make_shared<camera::CameraParameter>();
  param->name = "webcam";
  param->timeout_ms = kCameraTimeoutMs;

  camera_ = camera::CameraFactory::CreateCamera(kCameraType, param);
  if (!camera_) {
    LOG_ERROR("Failed to create camera (type: %s)", kCameraType);
    return false;
  }

  if (camera_->Initialize() != camera::CameraState::Status::kSucceeded) {
    LOG_ERROR("Failed to initialize camera");
    return false;
  }

  if (camera_->Open() != camera::CameraState::Status::kSucceeded) {
    LOG_ERROR("Failed to open camera");
    return false;
  }

  http_server_->Initialize(port, resources_path);
  http_server_->Start();
  LOG_INFO("HTTP server started on port %d", port);

  raw_image_publisher_.Initialize(http_server_, kRawStreamId);
  overlay_image_publisher_.Initialize(http_server_, kOverlayStreamId);
  LOG_INFO("ImagePublisher initialized (stream_id: %s)", kRawStreamId);
  LOG_INFO("ImagePublisher initialized (stream_id: %s)", kOverlayStreamId);

  detector_ = detector::DetectorFactory::CreateDetector("yolo26");
  if (!detector_) {
    LOG_WARN("Failed to create YOLO26 detector. Overlay stream will be skipped.");
  } else {
    detector::DetectorParameter detector_param;
    detector_param.model_path = YOONVISION_YOLO26_MODEL_PATH;
    detector_param.model_type = "yolo26";
    detector_param.confidence_threshold = 0.5f;
    detector_param.nms_threshold = 0.45f;
    detector_param.max_detections = 200;
    detector_param.input_width = 640;
    detector_param.input_height = 640;
    detector_param.letterbox = true;
    detector_param.normalize = true;
    detector_param.color_space = detector::ColorSpace::kRGB;
    detector_param.provider = compute::Processor::Provider::CPU;

    if (!std::filesystem::exists(detector_param.model_path)) {
      LOG_WARN("YOLO26 model file not found: %s. Overlay stream will be skipped.",
               detector_param.model_path.c_str());
      detector_.reset();
    } else if (!detector_->Initialize(detector_param)) {
      LOG_WARN("YOLO26 detector initialization failed. Overlay stream will be "
               "skipped.");
      detector_.reset();
    } else {
      LOG_INFO("YOLO26 detector initialized with model: %s",
               detector_param.model_path.c_str());
    }
  }

  subscriber_ = std::make_shared<FramePublisherSubscriber>(
      raw_image_publisher_, &overlay_image_publisher_, detector_.get(),
      kInferenceIntervalFrames);
  camera_->Subscribe(subscriber_);
  LOG_INFO("Camera initialized and subscriber registered");

  initialized_ = true;
  LOG_INFO("VisionApplication initialized successfully");
  return true;
}

bool VisionApplication::Run() {
  if (!initialized_) {
    LOG_ERROR("VisionApplication is not initialized");
    return false;
  }

  if (camera_->StreamOn() != camera::CameraState::Status::kSucceeded) {
    LOG_ERROR("Failed to start camera stream");
    return false;
  }

  LOG_INFO("Camera streaming started. Browse http://localhost to view.");

  while (!g_should_shutdown.load()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(kMainLoopSleepMs));
  }

  LOG_INFO("Shutdown signal received, exiting main loop...");
  return true;
}

void VisionApplication::Shutdown() {
  if (!initialized_) {
    return;
  }

  LOG_INFO("Shutting down VisionApplication...");

  if (camera_) {
    camera_->StreamOff();
    if (subscriber_) {
      camera_->Unsubscribe(subscriber_);
    }
    camera_->Close();
    camera_->Finalize();
    LOG_INFO("Camera finalized");
  }

  if (http_server_) {
    http_server_->Stop();
    LOG_INFO("HTTP server stopped");
  }

  initialized_ = false;
  LOG_INFO("VisionApplication terminated");
}

}  // namespace yoonvision::viewer
