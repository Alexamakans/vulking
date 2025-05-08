#pragma once
#include "../common.hpp"
#include "PhysicalDevice.hpp"
#include <optional>
#include <vector>

namespace Vulking {
class Device {
public:
  Device(PhysicalDevice physicalDevice);
  ~Device();

  operator VkDevice() const;

private:
  VkDevice device;
  PhysicalDevice physicalDevice;

  void createLogicalDevice(uint32_t graphicsQueueFamily,
                           uint32_t presentQueueFamily);
};
} // namespace Vulking
