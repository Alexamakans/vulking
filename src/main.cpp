#include "common.hpp"
#include "devices/GPU.hpp"
#include "wrappers/DebugMessenger.hpp"
#include "wrappers/Device.hpp"
#include "wrappers/Instance.hpp"
#include "wrappers/RenderPass.hpp"
#include "wrappers/Surface.hpp"
#include "wrappers/SwapChain.hpp"
#include <vulkan/vulkan_core.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

const int WIDTH = 800;
const int HEIGHT = 600;

#define CONCAT_IMPL(a, b) a##b
#define CONCAT(a, b) CONCAT_IMPL(a, b)
#define DECLARE_VAR(type, name)                                                \
  alignas(type) std::byte __##name##_buf[sizeof(type)];                        \
  type *__p_##name = nullptr;                                                  \
  Vulking::Ptr<type> name;

#define DEFINE_VAR(type, name, ...)                                            \
  __p_##name = new (__##name##_buf) type(__VA_ARGS__);                         \
  name = Vulking::Ptr<type>(__p_##name);

class MyApplication {
public:
  GLFWwindow *window;
  Vulking::Instance instance;
  Vulking::DebugMessenger debugMessenger;

  DECLARE_VAR(Vulking::Surface, surface)
  DECLARE_VAR(Vulking::GPU, gpu)
  DECLARE_VAR(Vulking::RenderPass, renderPass)
  DECLARE_VAR(Vulking::SwapChain, swapChain)

  MyApplication()
      : window(createWindow()), instance(getRequiredExtensions(), true),
        debugMessenger(instance) {
    DEFINE_VAR(Vulking::Surface, surface, instance, createSurface());
    DEFINE_VAR(Vulking::GPU, gpu, instance, surface);
    DEFINE_VAR(Vulking::RenderPass, renderPass, gpu->device,
               VK_FORMAT_R32G32B32_SFLOAT, findDepthFormat(),
               gpu->physicalDevice.getMsaaSamples());
    DEFINE_VAR(Vulking::SwapChain, swapChain, gpu->physicalDevice, gpu->device,
               surface);
  }

  void run() { release(); }

private:
  void release() {
    swapChain->release();
    renderPass->release();
    gpu->release();
    surface->release();
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

  VkSurfaceKHR createSurface() {
    VkSurfaceKHR vkSurface;
    CHK(glfwCreateWindowSurface(instance, window, allocator, &vkSurface),
        "failed to create surface");
    return vkSurface;
  }

  VkFormat findDepthFormat() {
    return findSupportedFormat(
        {
            VK_FORMAT_D32_SFLOAT,
            VK_FORMAT_D32_SFLOAT_S8_UINT,
            VK_FORMAT_D24_UNORM_S8_UINT,
        },
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
  }

  VkFormat findSupportedFormat(const std::vector<VkFormat> &candidates,
                               VkImageTiling tiling,
                               VkFormatFeatureFlags features) {
    for (VkFormat format : candidates) {
      VkFormatProperties props;
      vkGetPhysicalDeviceFormatProperties(gpu->physicalDevice, format, &props);

      if (tiling == VK_IMAGE_TILING_LINEAR &&
          (props.linearTilingFeatures & features) == features) {
        return format;
      } else if (tiling == VK_IMAGE_TILING_OPTIMAL &&
                 (props.optimalTilingFeatures & features) == features) {
        return format;
      }
    }

    throw std::runtime_error("failed to find supported format!");
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
