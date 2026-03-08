#include "yoonvision_viewer/frame_publisher_subscriber.hpp"

namespace yoonvision::viewer {

FramePublisherSubscriber::FramePublisherSubscriber(
    http::ImagePublisher& image_publisher)
    : image_publisher_(image_publisher) {}

void FramePublisherSubscriber::OnFrame(const Image::Ptr& frame) {
  if (!frame) {
    return;
  }

  image_publisher_.SetImage(frame);
  image_publisher_.Publish();
}

}  // namespace yoonvision::viewer