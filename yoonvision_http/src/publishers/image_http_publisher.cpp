#include "yoonvision_http/http_publisher.hpp"
#include "log.hpp"

namespace yoonvision {
namespace http {

void ImagePublisher::Initialize(const std::shared_ptr<HTTPServer>& server,
                                const std::string& stream_id) {
  server_ = server;
  stream_id_ = stream_id;
  LOG_INFO("HTTP image publisher initialized: %s", stream_id.c_str());
}

void ImagePublisher::SetImage(Image::Ptr image) {
  image_ = std::move(image);
}

bool ImagePublisher::Publish() {
  if (!server_) {
    LOG_WARN("HTTP server not set");
    return false;
  }

  if (!image_) {
    LOG_WARN("No image to publish");
    return false;
  }

  server_->RegisterStream(stream_id_, image_);
  return true;
}

}  // namespace http
}  // namespace yoonvision
