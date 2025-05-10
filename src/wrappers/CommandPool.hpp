#pragma once

#include "../common.hpp"
#include "Device.hpp"
#include "PhysicalDevice.hpp"
#include <vulkan/vulkan_core.h>

namespace Vulking {
class CommandPool {
public:
  CommandPool(const PhysicalDevice &physicalDevice, const Device &device);

  operator VkCommandPool() const { return commandPool; }

private:
  const PhysicalDevice &physicalDevice;
  const Device &device;

  VkCommandPool commandPool;

  VkCommandPool createCommandPool();
};
} // namespace Vulking
