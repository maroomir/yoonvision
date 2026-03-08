#include "yooncamera/avfcam/avf_camera.hpp"

#import <AVFoundation/AVFoundation.h>
#import <CoreMedia/CoreMedia.h>
#import <CoreVideo/CoreVideo.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <fstream>
#include <sstream>
#include <thread>

#include "log.hpp"
#include "yoonvision/image.hpp"
#include "yoonvision/image_builder.hpp"

@interface AvfFrameDelegate
    : NSObject <AVCaptureVideoDataOutputSampleBufferDelegate>
- (instancetype)initWithCamera:
    (yoonvision::camera::avfcam::AvfCamera*)camera;
- (void)invalidateCamera;
@end

@implementation AvfFrameDelegate {
  std::atomic<yoonvision::camera::avfcam::AvfCamera*> camera_;
}

- (instancetype)initWithCamera:
    (yoonvision::camera::avfcam::AvfCamera*)camera {
  if (self = [super init]) {
    camera_.store(camera, std::memory_order_release);
  }
  return self;
}

- (void)invalidateCamera {
  camera_.store(nullptr, std::memory_order_release);
}

- (void)captureOutput:(AVCaptureOutput*)output
    didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer
           fromConnection:(AVCaptureConnection*)connection {
  (void)output;
  (void)connection;
  auto* camera = camera_.load(std::memory_order_acquire);
  if (camera) {
    camera->OnSampleBuffer(static_cast<void*>(sampleBuffer));
  }
}

- (void)captureOutput:(AVCaptureOutput*)output
    didDropSampleBuffer:(CMSampleBufferRef)sampleBuffer
         fromConnection:(AVCaptureConnection*)connection {
  (void)output;
  (void)sampleBuffer;
  (void)connection;
}

@end

struct AvfCameraImpl {
  AVCaptureSession*            session      = nil;
  AVCaptureDeviceInput*        device_input = nil;
  AVCaptureVideoDataOutput*    video_output = nil;
  AvfFrameDelegate*            delegate     = nil;
  dispatch_queue_t             capture_queue = nullptr;

  AvfCameraImpl() {
    capture_queue = dispatch_queue_create(
        "yooncamera.avfcam.capture", DISPATCH_QUEUE_SERIAL);
  }

  ~AvfCameraImpl() {
    session      = nil;
    device_input = nil;
    video_output = nil;
    delegate     = nil;
  }
};

namespace yoonvision::camera::avfcam {

namespace {

inline byte ClampToByte(int value) {
  if (value < 0) {
    return static_cast<byte>(0);
  }
  if (value > 255) {
    return static_cast<byte>(255);
  }
  return static_cast<byte>(value);
}

Image::Ptr BuildRgbImage(std::vector<byte>&& rgb_data,
                         std::size_t width,
                         std::size_t height) {
  Image built = ImageBuilder()
                    .FromBuffer(rgb_data, width, height,
                                Image::ImageFormat::kRgbMixed)
                    .Build();
  return std::make_shared<Image>(std::move(built));
}

Image::Ptr ConvertBgraToImage(CVPixelBufferRef pixel_buffer) {
  CVPixelBufferLockBaseAddress(pixel_buffer, kCVPixelBufferLock_ReadOnly);

  const std::size_t width  = CVPixelBufferGetWidth(pixel_buffer);
  const std::size_t height = CVPixelBufferGetHeight(pixel_buffer);
  const std::size_t stride = CVPixelBufferGetBytesPerRow(pixel_buffer);
  const uint8_t* src =
      static_cast<const uint8_t*>(CVPixelBufferGetBaseAddress(pixel_buffer));

  if (!src || width == 0 || height == 0) {
    CVPixelBufferUnlockBaseAddress(pixel_buffer, kCVPixelBufferLock_ReadOnly);
    return nullptr;
  }

  // #region agent log
  {
    const std::size_t width_x4 = width * 4u;
    const bool stride_ok = (stride >= width_x4);
    const bool is_planar = CVPixelBufferIsPlanar(pixel_buffer);
    auto ts = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    std::ostringstream os;
    os << "{\"sessionId\":\"deb9c1\",\"timestamp\":" << ts
       << ",\"location\":\"avf_camera.mm:ConvertBgraToImage\",\"message\":\"BGRA layout\",\"data\":{"
       << "\"width\":" << width << ",\"height\":" << height
       << ",\"stride\":" << stride << ",\"width_x4\":" << width_x4
       << ",\"stride_ok\":" << (stride_ok ? "true" : "false")
       << ",\"is_planar\":" << (is_planar ? "true" : "false")
       << "},\"hypothesisId\":\"A,B,E\"}\n";
    std::ofstream f("/Users/maroomir/Git/maroomir/yoonvision/.cursor/debug-deb9c1.log", std::ios::app);
    if (f) f << os.str();
  }
  {
    auto ts = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    const std::size_t h = height;
    const std::size_t r0 = 0, r1 = (h >= 2) ? (h / 2 - 1) : 0, r2 = h / 2, r3 = (h >= 1) ? (h - 1) : 0;
    auto sample = [&](std::size_t row) -> std::string {
      const uint8_t* row_ptr = src + row * stride;
      int b = row_ptr[0], g = row_ptr[1], r = row_ptr[2];
      std::ostringstream o; o << "[" << r << "," << g << "," << b << "]"; return o.str();
    };
    std::ostringstream os;
    os << "{\"sessionId\":\"deb9c1\",\"timestamp\":" << ts
       << ",\"location\":\"avf_camera.mm:ConvertBgraToImage\",\"message\":\"src sample rows\",\"data\":{"
       << "\"row0\":" << sample(r0) << ",\"row_h2m1\":" << sample(r1)
       << ",\"row_h2\":" << sample(r2) << ",\"row_last\":" << sample(r3)
       << "},\"hypothesisId\":\"C\"}\n";
    std::ofstream f("/Users/maroomir/Git/maroomir/yoonvision/.cursor/debug-deb9c1.log", std::ios::app);
    if (f) f << os.str();
  }
  // #endregion

  const std::size_t rgb_stride = width * 3;
  std::vector<byte> rgb_data(rgb_stride * height);

  for (std::size_t row = 0; row < height; ++row) {
    const uint8_t* src_row = src + row * stride;
    byte*          dst_row = rgb_data.data() + row * rgb_stride;
    for (std::size_t col = 0; col < width; ++col) {
      dst_row[col * 3 + 0] =
          static_cast<byte>(src_row[col * 4 + 2]);  // R ← BGRA[2]
      dst_row[col * 3 + 1] =
          static_cast<byte>(src_row[col * 4 + 1]);  // G ← BGRA[1]
      dst_row[col * 3 + 2] =
          static_cast<byte>(src_row[col * 4 + 0]);  // B ← BGRA[0]
    }
  }

  // #region agent log
  {
    const std::size_t h = height;
    const std::size_t r2 = h / 2;
    const uint8_t* src_row2 = src + r2 * stride;
    auto ts = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    std::ostringstream os;
    os << "{\"sessionId\":\"deb9c1\",\"timestamp\":" << ts
       << ",\"location\":\"avf_camera.mm:ConvertBgraToImage\",\"message\":\"src row_h2 after copy\",\"data\":{"
       << "\"r\":" << (int)src_row2[2] << ",\"g\":" << (int)src_row2[1] << ",\"b\":" << (int)src_row2[0]
       << "},\"hypothesisId\":\"D\"}\n";
    std::ofstream f("/Users/maroomir/Git/maroomir/yoonvision/.cursor/debug-deb9c1.log", std::ios::app);
    if (f) f << os.str();
  }
  {
    const std::size_t h = height;
    auto samp = [&](std::size_t row) -> std::string {
      const byte* p = rgb_data.data() + row * rgb_stride;
      std::ostringstream o; o << "[" << (int)p[0] << "," << (int)p[1] << "," << (int)p[2] << "]"; return o.str();
    };
    std::size_t r0 = 0, r1 = (h >= 2) ? (h/2 - 1) : 0, r2 = h/2, r3 = (h >= 1) ? (h - 1) : 0;
    auto ts = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    std::ostringstream os;
    os << "{\"sessionId\":\"deb9c1\",\"timestamp\":" << ts
       << ",\"location\":\"avf_camera.mm:ConvertBgraToImage\",\"message\":\"dst sample rows\",\"data\":{"
       << "\"row0\":" << samp(r0) << ",\"row_h2m1\":" << samp(r1)
       << ",\"row_h2\":" << samp(r2) << ",\"row_last\":" << samp(r3)
       << "},\"hypothesisId\":\"C\"}\n";
    std::ofstream f("/Users/maroomir/Git/maroomir/yoonvision/.cursor/debug-deb9c1.log", std::ios::app);
    if (f) f << os.str();
  }
  // #endregion

  CVPixelBufferUnlockBaseAddress(pixel_buffer, kCVPixelBufferLock_ReadOnly);
  return BuildRgbImage(std::move(rgb_data), width, height);
}

Image::Ptr ConvertNv12ToImage(CVPixelBufferRef pixel_buffer,
                              bool full_range) {
  CVPixelBufferLockBaseAddress(pixel_buffer, kCVPixelBufferLock_ReadOnly);

  const std::size_t width = CVPixelBufferGetWidth(pixel_buffer);
  const std::size_t height = CVPixelBufferGetHeight(pixel_buffer);

  if (CVPixelBufferGetPlaneCount(pixel_buffer) < 2 || width == 0 ||
      height == 0) {
    CVPixelBufferUnlockBaseAddress(pixel_buffer, kCVPixelBufferLock_ReadOnly);
    return nullptr;
  }

  const uint8_t* y_plane =
      static_cast<const uint8_t*>(CVPixelBufferGetBaseAddressOfPlane(
          pixel_buffer, 0));
  const uint8_t* uv_plane =
      static_cast<const uint8_t*>(CVPixelBufferGetBaseAddressOfPlane(
          pixel_buffer, 1));
  const std::size_t y_stride = CVPixelBufferGetBytesPerRowOfPlane(pixel_buffer, 0);
  const std::size_t uv_stride =
      CVPixelBufferGetBytesPerRowOfPlane(pixel_buffer, 1);

  if (!y_plane || !uv_plane) {
    CVPixelBufferUnlockBaseAddress(pixel_buffer, kCVPixelBufferLock_ReadOnly);
    return nullptr;
  }

  std::vector<byte> rgb_data(width * height * 3);

  for (std::size_t row = 0; row < height; ++row) {
    const uint8_t* y_row = y_plane + row * y_stride;
    const uint8_t* uv_row = uv_plane + (row / 2) * uv_stride;
    byte* dst_row = rgb_data.data() + row * width * 3;

    for (std::size_t col = 0; col < width; ++col) {
      const int y = static_cast<int>(y_row[col]);
      const std::size_t uv_index = (col / 2) * 2;
      const int u = static_cast<int>(uv_row[uv_index + 0]);  // Cb
      const int v = static_cast<int>(uv_row[uv_index + 1]);  // Cr

      int r = 0;
      int g = 0;
      int b = 0;

      if (full_range) {
        const int d = u - 128;
        const int e = v - 128;
        r = y + ((359 * e) >> 8);
        g = y - ((88 * d + 183 * e) >> 8);
        b = y + ((454 * d) >> 8);
      } else {
        const int c = std::max(0, y - 16);
        const int d = u - 128;
        const int e = v - 128;
        r = (298 * c + 409 * e + 128) >> 8;
        g = (298 * c - 100 * d - 208 * e + 128) >> 8;
        b = (298 * c + 516 * d + 128) >> 8;
      }

      dst_row[col * 3 + 0] = ClampToByte(r);
      dst_row[col * 3 + 1] = ClampToByte(g);
      dst_row[col * 3 + 2] = ClampToByte(b);
    }
  }

  CVPixelBufferUnlockBaseAddress(pixel_buffer, kCVPixelBufferLock_ReadOnly);
  return BuildRgbImage(std::move(rgb_data), width, height);
}

}  // namespace

AvfCamera::AvfCamera(std::shared_ptr<CameraParameter> param)
    : param_(std::move(param)),
      impl_(new AvfCameraImpl()) {}

AvfCamera::~AvfCamera() {
  if (!IsFinalized()) {
    Finalize();
  }
  delete impl_;
  impl_ = nullptr;
}

CameraState::Status AvfCamera::Initialize() {
  if (IsInitialized()) {
    return CameraState::Status::kSucceeded;
  }

  @autoreleasepool {
    impl_->session = [[AVCaptureSession alloc] init];
    [impl_->session setSessionPreset:AVCaptureSessionPreset1280x720];
  }

  state_.initialize = CameraState::Status::kSucceeded;
  return state_.initialize;
}

void AvfCamera::Finalize() {
  Close();

  @autoreleasepool {
    impl_->session      = nil;
    impl_->device_input = nil;
    impl_->video_output = nil;
    impl_->delegate     = nil;
  }

  state_.finalize = CameraState::Status::kSucceeded;
}

CameraState::Status AvfCamera::Open() {
  if (!IsInitialized()) {
    state_.open_camera = CameraState::Status::kFailed;
    return state_.open_camera;
  }

  @autoreleasepool {
    AVCaptureDevice* device = nil;
    if (param_ && !param_->device_path.empty()) {
      NSString* uid = [NSString stringWithUTF8String:param_->device_path.c_str()];
      device = [AVCaptureDevice deviceWithUniqueID:uid];
    }
    if (!device) {
      if (@available(macOS 14.0, *)) {
        // macOS 14+ : AVCaptureDeviceTypeExternal 사용
        AVCaptureDeviceDiscoverySession* discovery =
            [AVCaptureDeviceDiscoverySession
                discoverySessionWithDeviceTypes:@[
                  AVCaptureDeviceTypeBuiltInWideAngleCamera,
                  AVCaptureDeviceTypeExternal
                ]
                                      mediaType:AVMediaTypeVideo
                                       position:AVCaptureDevicePositionUnspecified];
        device = discovery.devices.firstObject;
      } else if (@available(macOS 10.15, *)) {
        // macOS 10.15 ~ 13 : ExternalUnknown (deprecated in 14, but still works)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
        AVCaptureDeviceDiscoverySession* discovery =
            [AVCaptureDeviceDiscoverySession
                discoverySessionWithDeviceTypes:@[
                  AVCaptureDeviceTypeBuiltInWideAngleCamera,
                  AVCaptureDeviceTypeExternalUnknown
                ]
                                      mediaType:AVMediaTypeVideo
                                       position:AVCaptureDevicePositionUnspecified];
#pragma clang diagnostic pop
        device = discovery.devices.firstObject;
      } else {
        device = [AVCaptureDevice defaultDeviceWithMediaType:AVMediaTypeVideo];
      }
    }

    if (!device) {
      state_.open_camera = CameraState::Status::kFailed;
      return state_.open_camera;
    }

    NSError* error = nil;
    AVCaptureDeviceInput* input =
        [AVCaptureDeviceInput deviceInputWithDevice:device error:&error];
    if (error || !input) {
      state_.open_camera = CameraState::Status::kFailed;
      return state_.open_camera;
    }

    if (![impl_->session canAddInput:input]) {
      state_.open_camera = CameraState::Status::kFailed;
      return state_.open_camera;
    }
    [impl_->session addInput:input];
    impl_->device_input = input;

    AVCaptureVideoDataOutput* output =
        [[AVCaptureVideoDataOutput alloc] init];
    output.videoSettings = @{
      (NSString*)kCVPixelBufferPixelFormatTypeKey :
          @(kCVPixelFormatType_32BGRA)
    };
    output.alwaysDiscardsLateVideoFrames = YES;

    impl_->delegate = [[AvfFrameDelegate alloc] initWithCamera:this];
    [output setSampleBufferDelegate:impl_->delegate
                              queue:impl_->capture_queue];

    if (![impl_->session canAddOutput:output]) {
      state_.open_camera = CameraState::Status::kFailed;
      return state_.open_camera;
    }
    [impl_->session addOutput:output];
    impl_->video_output = output;
  }

  state_.open_camera = CameraState::Status::kSucceeded;
  return state_.open_camera;
}

void AvfCamera::Close() {
  StreamOff();

  @autoreleasepool {
    if (impl_->video_output) {
      [impl_->video_output setSampleBufferDelegate:nil queue:nullptr];
    }
    if (impl_->delegate) {
      [impl_->delegate invalidateCamera];
    }

    if (impl_->capture_queue) {
      dispatch_sync(impl_->capture_queue, ^{
      });
    }

    if (impl_->session) {
      if (impl_->device_input &&
          [impl_->session.inputs containsObject:impl_->device_input]) {
        [impl_->session removeInput:impl_->device_input];
      }
      if (impl_->video_output &&
          [impl_->session.outputs containsObject:impl_->video_output]) {
        [impl_->session removeOutput:impl_->video_output];
      }
    }
    impl_->device_input = nil;
    impl_->video_output = nil;
    impl_->delegate     = nil;
  }

  state_.close_camera = CameraState::Status::kSucceeded;
}

CameraState::Status AvfCamera::StreamOn() {
  if (!impl_->session || !impl_->video_output) {
    state_.stream_on = CameraState::Status::kFailed;
    return state_.stream_on;
  }

  @autoreleasepool {
    if (![impl_->session isRunning]) {
      [impl_->session startRunning];
    }
  }

  state_.stream_on = CameraState::Status::kSucceeded;
  return state_.stream_on;
}

void AvfCamera::StreamOff() {
  @autoreleasepool {
    if (impl_->session && [impl_->session isRunning]) {
      [impl_->session stopRunning];
    }
  }
  state_.stream_off = CameraState::Status::kSucceeded;
}

CameraState::Status AvfCamera::Capture() {
  if (!impl_->session || !impl_->video_output) {
    state_.capture = CameraState::Status::kFailed;
    return state_.capture;
  }

  const bool was_running = [impl_->session isRunning];
  if (!was_running) {
    @autoreleasepool {
      [impl_->session startRunning];
    }
  }

  const uint16_t timeout_ms = param_ ? param_->timeout_ms : 1000;
  const auto deadline =
      std::chrono::steady_clock::now() +
      std::chrono::milliseconds(timeout_ms);

  while (std::chrono::steady_clock::now() < deadline) {
    {
      std::lock_guard<std::mutex> lock(frame_mutex_);
      if (latest_frame_) {
        break;
      }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }

  if (!was_running) {
    @autoreleasepool {
      [impl_->session stopRunning];
    }
  }

  std::lock_guard<std::mutex> lock(frame_mutex_);
  state_.capture = latest_frame_ ? CameraState::Status::kSucceeded
                                 : CameraState::Status::kFailed;
  if (state_.capture == CameraState::Status::kSucceeded) {
    ++state_.monitor.success_frame_counter;
  } else {
    ++state_.monitor.failed_frame_counter;
  }
  return state_.capture;
}

void AvfCamera::SetupParameter(std::shared_ptr<CameraParameter> param) {
  param_ = std::move(param);
}

Image::Ptr AvfCamera::GetCaptureFrame() {
  std::lock_guard<std::mutex> lock(frame_mutex_);
  return latest_frame_;
}

bool AvfCamera::IsInitialized() const {
  return state_.initialize == CameraState::Status::kSucceeded;
}

bool AvfCamera::IsFinalized() const {
  return state_.finalize == CameraState::Status::kSucceeded;
}

CameraState AvfCamera::GetCameraState() const {
  return state_;
}

void AvfCamera::Subscribe(IFrameSubscriber::Ptr subscriber) {
  if (!subscriber) return;
  std::lock_guard<std::mutex> lock(subscriber_mutex_);
  subscribers_.push_back(std::move(subscriber));
}

void AvfCamera::Unsubscribe(IFrameSubscriber::Ptr subscriber) {
  if (!subscriber) return;
  std::lock_guard<std::mutex> lock(subscriber_mutex_);
  subscribers_.erase(
      std::remove(subscribers_.begin(), subscribers_.end(), subscriber),
      subscribers_.end());
}

void AvfCamera::NotifySubscribers(const Image::Ptr& frame) {
  std::lock_guard<std::mutex> lock(subscriber_mutex_);
  for (const auto& sub : subscribers_) {
    if (sub) {
      sub->OnFrame(frame);
    }
  }
}

void AvfCamera::OnSampleBuffer(void* sample_buffer_ref) {
  auto* sample_buffer = static_cast<CMSampleBufferRef>(sample_buffer_ref);
  if (!sample_buffer) return;

  CVPixelBufferRef pixel_buffer =
      CMSampleBufferGetImageBuffer(sample_buffer);
  if (!pixel_buffer) return;

  const OSType pixel_format = CVPixelBufferGetPixelFormatType(pixel_buffer);
  static OSType logged_pixel_format = 0;
  if (logged_pixel_format != pixel_format) {
    LOG_INFO("AVF pixel format changed: 0x%08X", pixel_format);
    logged_pixel_format = pixel_format;
  }

  Image::Ptr frame;
  switch (pixel_format) {
    case kCVPixelFormatType_32BGRA:
      frame = ConvertBgraToImage(pixel_buffer);
      break;
    case kCVPixelFormatType_420YpCbCr8BiPlanarFullRange:
      frame = ConvertNv12ToImage(pixel_buffer, true);
      break;
    case kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange:
      frame = ConvertNv12ToImage(pixel_buffer, false);
      break;
    default:
      LOG_WARN2("Unsupported pixel format: 0x%08X", pixel_format);
      frame = nullptr;
      break;
  }

  if (!frame) {
    std::lock_guard<std::mutex> lock(frame_mutex_);
    ++state_.monitor.failed_frame_counter;
    return;
  }

  {
    std::lock_guard<std::mutex> lock(frame_mutex_);
    latest_frame_ = frame;
    ++state_.monitor.success_frame_counter;
  }

  NotifySubscribers(frame);
}

}  // namespace yoonvision::camera::avfcam
