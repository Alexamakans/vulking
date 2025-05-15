#pragma once

#include "Common.hpp"
#include "PhysicalDevice.hpp"
#include <vulkan/vulkan_core.h>

namespace Vulking {
class Device {
public:
  Device() = default;
  Device(PhysicalDevice physicalDevice);

  MOVE_ONLY(Device);

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
