#pragma once

#include "../common.hpp"

#include "../wrappers/Device.hpp"
#include "../wrappers/PhysicalDevice.hpp"
#include "../wrappers/Queue.hpp"
#include <vulkan/vulkan_core.h>

namespace Vulking {
class GPU {
public:
  GPU(const Instance &instance, const Surface &surface);

  void release() { device.release(); }

  operator VkPhysicalDevice() const { return physicalDevice; }
  operator VkDevice() const { return device; }

  uint32_t getGraphicsQueueFamily() const;
  uint32_t getPresentQueueFamily() const;

  PhysicalDevice physicalDevice;
  Device device;

private:
  const Instance &instance;
  const Surface &surface;

  const Queue graphicsQueue;
  const Queue presentQueue;

  uint32_t graphicsQueueFamily{};
  uint32_t presentQueueFamily{};
};
} // namespace Vulking
