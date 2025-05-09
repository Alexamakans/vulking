#pragma once
#include "../common.hpp"
#include "PhysicalDevice.hpp"
#include <optional>
#include <vector>

namespace Vulking {
class Device {
public:
  Device() = default;
  Device(Device &) = default;
  Device(PhysicalDevice physicalDevice);
  ~Device();

  Device &operator=(const Device &other) {
    device = other.device;
    physicalDevice = other.physicalDevice;
    return *this;
  }
  operator VkDevice() const;

private:
  VkDevice device;
  PhysicalDevice physicalDevice;

  void createLogicalDevice(uint32_t graphicsQueueFamily,
                           uint32_t presentQueueFamily);
};
} // namespace Vulking
