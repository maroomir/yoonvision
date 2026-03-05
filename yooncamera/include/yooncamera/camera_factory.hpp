#ifndef YOONCAMERA_CAMERA_FACTORY_HPP_
#define YOONCAMERA_CAMERA_FACTORY_HPP_

#include <functional>
#include <memory>
#include <string>

#include "yooncamera/camera.hpp"

namespace yoonvision::camera {

using CameraCreator =
    std::function<ICamera::Ptr(std::shared_ptr<CameraParameter>)>;
using ContextManagerCreator =
    std::function<ICameraContextManager::Ptr()>;

class CameraFactory {
 public:
  static void RegisterCameraCreator(const std::string& camera_type,
                                    CameraCreator creator);
  static void RegisterContextManagerCreator(const std::string& camera_type,
                                            ContextManagerCreator creator);

  static ICamera::Ptr CreateCamera(const std::string& camera_type, 
                                   std::shared_ptr<CameraParameter> param);
  static ICameraContextManager::Ptr CreateContextManager(const std::string& camera_type);

  static bool HasCameraCreator(const std::string& camera_type);
  static bool HasContextManagerCreator(const std::string& camera_type);
  static void ClearAllCreators();
};

}  // namespace yoonvision::camera

#endif  // YOONCAMERA_CAMERA_FACTORY_HPP_
