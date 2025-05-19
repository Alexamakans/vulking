#pragma once

#include "Common.hpp"
#include "PhysicalDevice.hpp"

namespace Vulking {
class Device {
public:
  MOVE_ONLY(Device);

  Device() = default;
  Device(PhysicalDevice physicalDevice);

  void destroy();

  operator VkDevice() const;

  const PhysicalDevice &getPhysical() const;

private:
  PhysicalDevice physicalDevice;
  VkDevice device;

  VkQueue graphicsQueue;
  VkQueue presentQueue;
};
} // namespace Vulking
