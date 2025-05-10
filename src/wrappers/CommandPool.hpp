#pragma once

#include "../common.hpp"
#include "Device.hpp"
#include "PhysicalDevice.hpp"
#include <vulkan/vulkan_core.h>

namespace Vulking {
class CommandPool {
public:
  CommandPool(const PhysicalDevice &physicalDevice, const Device &device);

private:
  const PhysicalDevice &physicalDevice;
  const Device &device;

  VkCommandPool createCommandPool();
};
} // namespace Vulking
