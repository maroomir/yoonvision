#include "yooncamera/camera_factory.hpp"

#include <mutex>
#include <unordered_map>
#include <utility>

namespace yoonvision::camera {

namespace {

std::unordered_map<std::string, CameraCreator>& GetCameraCreators() {
  static std::unordered_map<std::string, CameraCreator> creators;
  return creators;
}

std::unordered_map<std::string, ContextManagerCreator>& GetContextCreators() {
  static std::unordered_map<std::string, ContextManagerCreator> creators;
  return creators;
}

std::mutex& GetFactoryMutex() {
  static std::mutex mutex;
  return mutex;
}

}  // namespace

void CameraFactory::RegisterCameraCreator(const std::string& camera_type,
                                          CameraCreator creator) {
  if (camera_type.empty() || !creator) {
    return;
  }
  std::lock_guard<std::mutex> lock(GetFactoryMutex());
  GetCameraCreators()[camera_type] = std::move(creator);
}

void CameraFactory::RegisterContextManagerCreator(const std::string& camera_type, 
                                                  ContextManagerCreator creator) {
  if (camera_type.empty() || !creator) {
    return;
  }
  std::lock_guard<std::mutex> lock(GetFactoryMutex());
  GetContextCreators()[camera_type] = std::move(creator);
}

ICamera::Ptr CameraFactory::CreateCamera(const std::string& camera_type, 
                                         std::shared_ptr<CameraParameter> param) {
  std::lock_guard<std::mutex> lock(GetFactoryMutex());
  const auto it = GetCameraCreators().find(camera_type);
  if (it == GetCameraCreators().end()) {
    return nullptr;
  }
  return it->second(std::move(param));
}

ICameraContextManager::Ptr CameraFactory::CreateContextManager(const std::string& camera_type) {
  std::lock_guard<std::mutex> lock(GetFactoryMutex());
  const auto it = GetContextCreators().find(camera_type);
  if (it == GetContextCreators().end()) {
    return nullptr;
  }
  return it->second();
}

bool CameraFactory::HasCameraCreator(const std::string& camera_type) {
  std::lock_guard<std::mutex> lock(GetFactoryMutex());
  return GetCameraCreators().find(camera_type) != GetCameraCreators().end();
}

bool CameraFactory::HasContextManagerCreator(const std::string& camera_type) {
  std::lock_guard<std::mutex> lock(GetFactoryMutex());
  return GetContextCreators().find(camera_type) != GetContextCreators().end();
}

void CameraFactory::ClearAllCreators() {
  std::lock_guard<std::mutex> lock(GetFactoryMutex());
  GetCameraCreators().clear();
  GetContextCreators().clear();
}

}  // namespace yoonvision::camera
