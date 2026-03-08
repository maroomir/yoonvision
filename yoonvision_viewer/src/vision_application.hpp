#ifndef YOONVISION_VIEWER_VISION_APPLICATION_HPP_
#define YOONVISION_VIEWER_VISION_APPLICATION_HPP_

#include <memory>
#include <string>

#include "yooncamera/camera.hpp"
#include "yoonvision_http/http_publisher.hpp"
#include "yoonvision_http/http_server.hpp"

namespace yoonvision::viewer {

class VisionApplication {
 public:
  VisionApplication();
  ~VisionApplication();

  bool Initialize(int port, const std::string& resources_path);
  bool Run();
  void Shutdown();

 private:
  class CameraSubscriber : public camera::IFrameSubscriber {
   public:
    explicit CameraSubscriber(VisionApplication* app);
    void OnFrame(const Image::Ptr& frame) override;

   private:
    VisionApplication* app_;
  };

  void on_frame(const Image::Ptr& frame);

  camera::ICamera::Ptr camera_;
  camera::IFrameSubscriber::Ptr subscriber_;

  std::shared_ptr<http::HTTPServer> http_server_;
  http::ImagePublisher image_publisher_;

  bool initialized_;
};

}  // namespace yoonvision::viewer

#endif  // YOONVISION_VIEWER_VISION_APPLICATION_HPP_
