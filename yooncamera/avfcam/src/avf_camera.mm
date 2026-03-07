#include "yooncamera/avfcam/avf_camera.hpp"

#import <AVFoundation/AVFoundation.h>
#import <CoreMedia/CoreMedia.h>
#import <CoreVideo/CoreVideo.h>

#include <algorithm>
#include <chrono>
#include <thread>

#include "yoonvision/image.hpp"
#include "yoonvision/image_builder.hpp"

@interface AvfFrameDelegate
    : NSObject <AVCaptureVideoDataOutputSampleBufferDelegate>
- (instancetype)initWithCamera:
    (yoonvision::camera::avfcam::AvfCamera*)camera;
@end

@implementation AvfFrameDelegate {
  yoonvision::camera::avfcam::AvfCamera* camera_;
}

- (instancetype)initWithCamera:
    (yoonvision::camera::avfcam::AvfCamera*)camera {
  if (self = [super init]) {
    camera_ = camera;
  }
  return self;
}

- (void)captureOutput:(AVCaptureOutput*)output
    didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer
           fromConnection:(AVCaptureConnection*)connection {
  (void)output;
  (void)connection;
  if (camera_) {
    camera_->OnSampleBuffer(static_cast<void*>(sampleBuffer));
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

  const std::size_t rgb_stride = width * 3;
  std::vector<byte> rgb_data(rgb_stride * height);

  for (std::size_t row = 0; row < height; ++row) {
    const uint8_t* src_row = src + row * stride;
    byte*          dst_row = rgb_data.data() + row * rgb_stride;
    for (std::size_t col = 0; col < width; ++col) {
      dst_row[col * 3 + 0] = static_cast<byte>(src_row[col * 4 + 2]);  // R ← BGRA[2]
      dst_row[col * 3 + 1] = static_cast<byte>(src_row[col * 4 + 1]);  // G ← BGRA[1]
      dst_row[col * 3 + 2] = static_cast<byte>(src_row[col * 4 + 0]);  // B ← BGRA[0]
    }
  }

  CVPixelBufferUnlockBaseAddress(pixel_buffer, kCVPixelBufferLock_ReadOnly);

  Image built = ImageBuilder()
      .FromBuffer(rgb_data, width, height, Image::ImageFormat::kRgbMixed)
      .Build();
  return std::make_shared<Image>(std::move(built));
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

  Image::Ptr frame = ConvertBgraToImage(pixel_buffer);
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
