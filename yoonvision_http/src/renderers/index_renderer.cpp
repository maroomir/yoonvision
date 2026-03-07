#include "yoonvision_http/page_renderer.hpp"
#include "yoonvision_http/resource_utils.hpp"
#include "log.hpp"
#include <sstream>

namespace yoonvision {
namespace http {

namespace {
constexpr char kResourceFileName[] = "/index.html";
}

IndexRenderer::IndexRenderer(GetStreamsCallback get_streams)
    : get_streams_(std::move(get_streams)) {}

void IndexRenderer::SetResourcePath(const std::string& path) {
  resources_path_ = path;
}

void IndexRenderer::Render(const httplib::Request& req, httplib::Response& res) {
  (void)req;

  std::map<std::string, Image::Ptr> streams = get_streams_();
  LOG_INFO("Index request - streams: %zu", streams.size());

  std::string html = generate_index_html(streams);
  res.set_content(html, "text/html");
}

std::string IndexRenderer::generate_index_html(
    const std::map<std::string, Image::Ptr>& streams) const {
  std::string html_template =
      resource_utils::LoadHTMLFile(resources_path_ + kResourceFileName);

  if (html_template.empty()) {
    LOG_ERROR("Failed to load index HTML template");
    return "<html><body><h1>Error: Failed to load template</h1></body></html>";
  }

  std::ostringstream streams_content;
  if (streams.empty()) {
    streams_content << "<p class=\"empty-state\">No streams available.</p>";
  } else {
    streams_content << "<div class=\"streams-grid\">\n";
    for (const auto& pair : streams) {
      const std::string& stream_id = pair.first;
      streams_content << "<div class=\"stream-container\">\n";
      streams_content << "<div class=\"stream-title\">" << stream_id
                      << "</div>\n";
      streams_content << "<img src=\"/stream/" << stream_id << "\" alt=\""
                      << stream_id << "\">\n";
      streams_content << "</div>\n";
    }
    streams_content << "</div>\n";
  }

  std::string html = resource_utils::AdjustTemplateToHTML(
      html_template, {{"streams_content", streams_content.str()}});

  return html;
}

}  // namespace http
}  // namespace yoonvision
