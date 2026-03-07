#include "yooncamera/avfcam/avf_camera.hpp"
#include "yooncamera/avfcam/avf_camera_context_manager.hpp"
#include "yooncamera/camera_factory.hpp"

namespace yoonvision::camera::avfcam {

namespace {

struct AvfCameraRegistrar {
  AvfCameraRegistrar() {
    CameraFactory::RegisterCameraCreator(
        "avfcam",
        [](std::shared_ptr<CameraParameter> param) -> ICamera::Ptr {
          return std::make_shared<AvfCamera>(std::move(param));
        });

    CameraFactory::RegisterContextManagerCreator(
        "avfcam",
        []() -> ICameraContextManager::Ptr {
          return std::make_shared<AvfCameraContextManager>();
        });
  }
};

static AvfCameraRegistrar kRegistrar;

}  // namespace

}  // namespace yoonvision::camera::avfcam
