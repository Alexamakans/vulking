#pragma once

#include "Common.hpp"
#include "Device.hpp"
#include "PhysicalDevice.hpp"
#include <vulkan/vulkan_core.h>

#define VULKING_MAKE_VERSION(major, minor, patch)                              \
  VK_MAKE_VERSION(major, minor, patch)

namespace Vulking {
class Engine {
public:
  Engine(GLFWwindow *window, const char *applicationInfo,
         uint32_t applicationVersion,
         std::vector<const char *> &requiredExtensions);

  ~Engine() {
    device.destroy();
    vkDestroyInstance(instance, ALLOCATOR);
  }

private:
  VkInstance createInstance(const char *applicationInfo,
                            uint32_t applicationVersion,
                            std::vector<const char *> &requiredExtensions);

  VkInstance instance;
  VkSurfaceKHR surface;

  PhysicalDevice physicalDevice;
  Device device;
  VkCommandPool commandPool;
  VkDescriptorPool descriptorPool;

  uint32_t graphicsQueueFamily;
  uint32_t presentQueueFamily;

  VkPhysicalDevice getSuitablePhysicalDevice(VkInstance instance);
  bool isDeviceSuitable(VkPhysicalDevice physicalDevice) const;
};
} // namespace Vulking
