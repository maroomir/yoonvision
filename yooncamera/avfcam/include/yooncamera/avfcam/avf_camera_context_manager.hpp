#ifndef YOONCAMERA_AVFCAM_AVF_CAMERA_CONTEXT_MANAGER_HPP_
#define YOONCAMERA_AVFCAM_AVF_CAMERA_CONTEXT_MANAGER_HPP_

#include <memory>

#include "yooncamera/camera_context_manager.hpp"

namespace yoonvision::camera::avfcam {

/// @brief AVFoundation 기반의 카메라 컨텍스트 매니저.
///        카메라 접근 권한 확인 및 세션 수준의 초기화/정리를 담당합니다.
class AvfCameraContextManager : public ICameraContextManager {
 public:
  AvfCameraContextManager();
  ~AvfCameraContextManager() override;

  /// @brief 카메라 접근 권한을 확인하고 컨텍스트를 초기화합니다.
  /// @return 권한 허용 및 초기화 성공 시 true
  bool Initialize() override;

  /// @brief 컨텍스트를 정리합니다.
  void Finalize() override;

  /// @return 초기화 완료 여부
  [[nodiscard]] bool IsInitialized() const override;

 private:
  bool initialized_;
};

}  // namespace yoonvision::camera::avfcam

#endif  // YOONCAMERA_AVFCAM_AVF_CAMERA_CONTEXT_MANAGER_HPP_
