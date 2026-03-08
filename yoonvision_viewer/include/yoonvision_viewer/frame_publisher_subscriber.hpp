#ifndef YOONVISION_VIEWER_FRAME_PUBLISHER_SUBSCRIBER_HPP_
#define YOONVISION_VIEWER_FRAME_PUBLISHER_SUBSCRIBER_HPP_

#include "yooncamera/camera.hpp"
#include "yoonvision_http/http_publisher.hpp"

namespace yoonvision::viewer {

class FramePublisherSubscriber : public camera::IFrameSubscriber {
 public:
  explicit FramePublisherSubscriber(http::ImagePublisher& image_publisher);

  void OnFrame(const Image::Ptr& frame) override;

 private:
  http::ImagePublisher& image_publisher_;
};

}  // namespace yoonvision::viewer

#endif  // YOONVISION_VIEWER_FRAME_PUBLISHER_SUBSCRIBER_HPP_