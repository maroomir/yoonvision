#include "yoonvision_viewer/vision_application.hpp"

#include <atomic>
#include <chrono>
#include <thread>

#include "yoonvision_viewer/frame_publisher_subscriber.hpp"
#include "log.hpp"
#include "yooncamera/camera_factory.hpp"

extern std::atomic<bool> g_should_shutdown;

namespace yoonvision::viewer {

namespace {
constexpr int kMainLoopSleepMs = 10;
constexpr char kStreamId[] = "webcam";
constexpr char kCameraType[] = "avfcam";
constexpr uint16_t kCameraTimeoutMs = 2000;
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

  subscriber_ = std::make_shared<FramePublisherSubscriber>(image_publisher_);
  camera_->Subscribe(subscriber_);
  LOG_INFO("Camera initialized and subscriber registered");
  http_server_->Initialize(port, resources_path);
  http_server_->Start();
  LOG_INFO("HTTP server started on port %d", port);

  image_publisher_.Initialize(http_server_, kStreamId);
  LOG_INFO("ImagePublisher initialized (stream_id: %s)", kStreamId);

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
