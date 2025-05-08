#include "devices/GPU.hpp"
#include "wrappers/DebugMessenger.hpp"
#include "wrappers/Instance.hpp"
#include "wrappers/Surface.hpp"
#include "wrappers/SwapChain.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

const int WIDTH = 800;
const int HEIGHT = 600;

class MyApplication {
public:
  GLFWwindow *window;
  Vulking::Instance instance;
  Vulking::DebugMessenger debugMessenger;
  Vulking::Surface surface;
  Vulking::GPU gpu;
  Vulking::SwapChain swapChain;

  MyApplication()
      : window(createWindow()), instance(true), debugMessenger(instance) {
    VkSurfaceKHR surface;
    glfwCreateWindowSurface(instance, window, allocator, &surface);
    this->surface = Vulking::Surface(instance, surface);
    gpu = Vulking::GPU();
  }

private:
  GLFWwindow *createWindow() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    GLFWwindow *window =
        glfwCreateWindow(WIDTH, HEIGHT, "Vulking", nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);
    // glfwSetFramebufferSizeCallback(window, onFramebufferResize);
    return window;
  }
};

int main() {
  MyApplication app;

  try {
    app.run();
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
