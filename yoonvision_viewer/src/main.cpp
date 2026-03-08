#include <csignal>
#include <atomic>
#include <cstdio>

#include "yoonvision_viewer/vision_application.hpp"

std::atomic<bool> g_should_shutdown(false);

static void signal_handler(int signum) {
  std::printf("\n[signal] Received signal %d, initiating shutdown...\n", signum);
  g_should_shutdown.store(true);
}

int main() {
  std::signal(SIGINT,  signal_handler);
  std::signal(SIGTERM, signal_handler);

  yoonvision::viewer::VisionApplication app;

  if (!app.Initialize(8080, YOONVISION_HTTP_RESOURCES_PATH)) {
    std::fprintf(stderr, "[error] Failed to initialize VisionApplication\n");
    return -1;
  }

  if (!app.Run()) {
    app.Shutdown();
    return -1;
  }

  app.Shutdown();
  return 0;
}
