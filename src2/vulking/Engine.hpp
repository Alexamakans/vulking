#pragma once

#include "Common.hpp"

#define VULKING_MAKE_VERSION(major, minor, patch)                              \
  VK_MAKE_VERSION(major, minor, patch)

namespace Vulking {
class Engine {
public:
  Engine(GLFWwindow *window, const char *applicationInfo,
         uint32_t applicationVersion,
         const std::vector<const char *> &requiredExtensions);

  ~Engine();

  static vk::CommandBuffer beginCommand();

  static vk::UniqueInstance instance;
  static vk::PhysicalDevice physicalDevice;
  static vk::UniqueDevice device;

  static vk::CommandPool commandPool;
  static vk::DescriptorPool descriptorPool;

private:
  vk::UniqueInstance
  createInstance(const char *applicationInfo, uint32_t applicationVersion,
                 const std::vector<const char *> &requiredExtensions);

  vk::UniqueDevice createDevice();

  vk::SurfaceKHR surface;

  uint32_t graphicsQueueFamily;
  uint32_t presentQueueFamily;

  vk::PhysicalDevice getSuitablePhysicalDevice();
  bool isDeviceSuitable(vk::PhysicalDevice physicalDevice) const;
};
} // namespace Vulking
