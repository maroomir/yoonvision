#ifndef YOONCAMERA_CAMERA_CONTEXT_MANAGER_HPP_
#define YOONCAMERA_CAMERA_CONTEXT_MANAGER_HPP_

namespace yoonvision::camera {

class ICameraContextManager {
 public:
  using Ptr = std::shared_ptr<ICameraContextManager>;
  
  virtual ~ICameraContextManager() = default;

  virtual bool Initialize() = 0;
  virtual void Finalize() = 0;
  [[nodiscard]] virtual bool IsInitialized() const = 0;
};

}  // namespace yoonvision::camera

#endif  // YOONCAMERA_CAMERA_CONTEXT_MANAGER_HPP_
