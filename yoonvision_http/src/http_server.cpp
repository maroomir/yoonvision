#include "yoonvision_http/http_server.hpp"
#include "yoonvision_http/page_renderer.hpp"
#include "log.hpp"
#include <sstream>

namespace yoonvision {
namespace http {

namespace {
constexpr int kDefaultPort = 8080;
}

HTTPServer::HTTPServer()
    : port_(kDefaultPort), server_(nullptr), running_(false) {}

HTTPServer::~HTTPServer() {
  Stop();
}

void HTTPServer::Initialize(int port, const std::string& resources_path) {
  port_ = port;
  resources_path_ = resources_path;
  server_ = std::make_unique<httplib::Server>();

  auto stream_renderer = std::make_unique<StreamRenderer>(
      std::bind(&HTTPServer::provide_stream, this, std::placeholders::_1),
      &running_);
  auto index_renderer = std::make_unique<IndexRenderer>(
      std::bind(&HTTPServer::provide_streams, this));

  stream_renderer->SetResourcePath(resources_path_);
  index_renderer->SetResourcePath(resources_path_);

  renderers_.push_back(std::move(stream_renderer));
  renderers_.push_back(std::move(index_renderer));

  server_->Get("/streams/api",
               [this](const httplib::Request& /*req*/, httplib::Response& res) {
                 handle_streams_api(res);
               });

  for (const auto& renderer : renderers_) {
    std::string path = renderer->GetPath();
    server_->Get(path,
                 [renderer_ptr = renderer.get()](const httplib::Request& req,
                                                 httplib::Response& res) {
                   renderer_ptr->Render(req, res);
                 });
  }

  LOG_INFO("HTTPServer initialized on port %d with resources path: %s", port_,
           resources_path_.c_str());
}

void HTTPServer::Start() {
  if (running_) {
    LOG_WARN("HTTPServer is already running");
    return;
  }

  running_ = true;
  server_thread_ = std::thread(&HTTPServer::server_task, this);

  LOG_INFO("HTTPServer started on port %d", port_);
}

void HTTPServer::Stop() {
  if (!running_) {
    return;
  }

  running_ = false;

  if (server_) {
    server_->stop();
  }

  if (server_thread_.joinable()) {
    server_thread_.join();
  }

  LOG_INFO("HTTPServer stopped");
}

bool HTTPServer::IsRunning() const {
  return running_;
}

void HTTPServer::RegisterStream(const std::string& stream_id, Image::Ptr frame) {
  std::lock_guard<std::mutex> lock(streams_mutex_);
  streams_[stream_id] = std::move(frame);
  LOG_INFO2("Registered stream: %s", stream_id.c_str());
}

void HTTPServer::UnregisterStream(const std::string& stream_id) {
  std::lock_guard<std::mutex> lock(streams_mutex_);
  streams_.erase(stream_id);
  LOG_INFO2("Unregistered stream: %s", stream_id.c_str());
}

std::vector<std::string> HTTPServer::GetStreamIds() const {
  std::lock_guard<std::mutex> lock(streams_mutex_);
  std::vector<std::string> ids;
  ids.reserve(streams_.size());
  for (const auto& pair : streams_) {
    ids.push_back(pair.first);
  }
  return ids;
}

Image::Ptr HTTPServer::provide_stream(const std::string& stream_id) const {
  std::lock_guard<std::mutex> lock(streams_mutex_);
  auto it = streams_.find(stream_id);
  if (it != streams_.end()) {
    return it->second;
  }
  return nullptr;
}

std::map<std::string, Image::Ptr> HTTPServer::provide_streams() const {
  std::lock_guard<std::mutex> lock(streams_mutex_);
  return streams_;
}

void HTTPServer::handle_streams_api(httplib::Response& res) {
  std::lock_guard<std::mutex> lock(streams_mutex_);
  std::ostringstream oss;
  oss << "{\"streams\": [";
  bool first = true;
  for (const auto& pair : streams_) {
    if (!first) {
      oss << ",";
    }
    oss << "\"" << pair.first << "\"";
    first = false;
  }
  oss << "]}";
  res.set_content(oss.str(), "application/json");
}

void HTTPServer::server_task() {
  if (!server_) {
    LOG_ERROR("Server is not initialized");
    return;
  }

  server_->listen("0.0.0.0", port_);
}

}  // namespace http
}  // namespace yoonvision
