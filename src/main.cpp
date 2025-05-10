#include "common.hpp"
#include "devices/GPU.hpp"
#include "helpers/VulkingUtil.hpp"
#include "wrappers/DebugMessenger.hpp"
#include "wrappers/Instance.hpp"
#include "wrappers/RenderPass.hpp"
#include "wrappers/Surface.hpp"
#include "wrappers/SwapChain.hpp"
#include <vulkan/vulkan_core.h>

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
  Vulking::RenderPass renderPass;
  Vulking::SwapChain swapChain;

  MyApplication()
      : window(createWindow()), instance(getRequiredExtensions(), true),
        debugMessenger(instance),
        surface(instance, createSurface(instance, window)),
        gpu(instance, surface),
        renderPass(gpu.device, VK_FORMAT_R32G32B32_SFLOAT,
                   VulkingUtil::findDepthFormat(gpu.physicalDevice),
                   gpu.physicalDevice.getMsaaSamples()),
        swapChain(gpu.physicalDevice, gpu.device, surface,
                  gpu.getRenderPass()) {}

  void run() { release(); }

private:
  void release() {
    swapChain.release();
    renderPass.release();
    gpu.release();
    surface.release();
    debugMessenger.release();
    instance.release();
    glfwDestroyWindow(window);
    glfwTerminate();
  }

  GLFWwindow *createWindow() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    GLFWwindow *window =
        glfwCreateWindow(WIDTH, HEIGHT, "Vulking", nullptr, nullptr);
    if (!window) {
      throw std::runtime_error("glfw window creation failed");
    }

    glfwSetWindowUserPointer(window, this);
    // glfwSetFramebufferSizeCallback(window, onFramebufferResize);
    return window;
  }

  std::vector<const char *> getRequiredExtensions() const {
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char *> extensions(glfwExtensions,
                                         glfwExtensions + glfwExtensionCount);

    if (enableValidationLayers) {
      extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
  }

  VkSurfaceKHR createSurface(const Vulking::Instance &instance,
                             GLFWwindow *window) {
    VkSurfaceKHR vkSurface;
    CHK(glfwCreateWindowSurface(instance, window, allocator, &vkSurface),
        "failed to create surface");
    return vkSurface;
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
