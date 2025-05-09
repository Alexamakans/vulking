#pragma once
#include "../common.hpp"
#include "PhysicalDevice.hpp"
#include <optional>
#include <vector>

namespace Vulking {
class Device {
public:
  Device(const PhysicalDevice &physicalDevice);
  void release() { vkDestroyDevice(device, allocator); }
  operator VkDevice() const;

private:
  const PhysicalDevice &physicalDevice;

  VkDevice device;

  void createLogicalDevice(uint32_t graphicsQueueFamily,
                           uint32_t presentQueueFamily);
};
} // namespace Vulking
