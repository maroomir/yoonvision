#include "yooncamera/avfcam/avf_camera_context_manager.hpp"

#import <AVFoundation/AVFoundation.h>

namespace yoonvision::camera::avfcam {

AvfCameraContextManager::AvfCameraContextManager() : initialized_(false) {}

AvfCameraContextManager::~AvfCameraContextManager() {
  if (initialized_) {
    Finalize();
  }
}

bool AvfCameraContextManager::Initialize() {
  if (initialized_) {
    return true;
  }

  // macOS 10.14+: 카메라 권한 확인 및 요청 
  AVAuthorizationStatus status =
      [AVCaptureDevice authorizationStatusForMediaType:AVMediaTypeVideo];

  if (status == AVAuthorizationStatusAuthorized) {
    initialized_ = true;
    return true;
  }

  if (status == AVAuthorizationStatusNotDetermined) {
    dispatch_semaphore_t semaphore = dispatch_semaphore_create(0);
    __block bool granted = false;

    [AVCaptureDevice requestAccessForMediaType:AVMediaTypeVideo
                             completionHandler:^(BOOL g) {
                               granted = g;
                               dispatch_semaphore_signal(semaphore);
                             }];

    dispatch_semaphore_wait(semaphore, DISPATCH_TIME_FOREVER);

    if (granted) {
      initialized_ = true;
      return true;
    }
  }

  // AVAuthorizationStatusDenied / AVAuthorizationStatusRestricted
  return false;
}

void AvfCameraContextManager::Finalize() {
  initialized_ = false;
}

bool AvfCameraContextManager::IsInitialized() const {
  return initialized_;
}

}  // namespace yoonvision::camera::avfcam
