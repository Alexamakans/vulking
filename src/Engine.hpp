#pragma once

#include "Common.hpp"
#include "Context.hpp"

namespace Vulking {
class Engine {
public:
  static Context &ctx() noexcept { return engineInstance->getContext(); }

  static Engine *engineInstance;

public:
  Engine(GLFWwindow *window, const char *applicationInfo,
         uint32_t applicationVersion,
         const std::vector<const char *> &requiredExtensions);

  Context &getContext() noexcept { return context; }

private:
  vk::UniqueInstance
  createInstance(const char *applicationInfo, uint32_t applicationVersion,
                 const std::vector<const char *> &requiredExtensions);

  vk::UniqueDevice createDevice();
  vk::UniqueSwapchainKHR createSwapchain();
  vk::UniqueCommandPool createCommandPool();

  vk::PhysicalDevice getSuitablePhysicalDevice();
  bool isDeviceSuitable(vk::PhysicalDevice physicalDevice) const;

  vk::SurfaceFormatKHR chooseSwapSurfaceFormat(
      const std::vector<vk::SurfaceFormatKHR> &availableFormats);

  vk::PresentModeKHR chooseSwapPresentMode(
      const std::vector<vk::PresentModeKHR> &availablePresentModes);

  vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities);

  Context context;
};
} // namespace Vulking
