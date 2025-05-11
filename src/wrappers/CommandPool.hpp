#pragma once

#include "../common.hpp"
#include "Device.hpp"
#include "PhysicalDevice.hpp"
#include <vulkan/vulkan_core.h>

namespace Vulking {
class CommandPool {
public:
  CommandPool(const PhysicalDevice &physicalDevice, const Device &device);

  void release() const { vkDestroyCommandPool(device, commandPool, allocator); }

  operator VkCommandPool() const { return commandPool; }

  void allocateBuffer(VkCommandBufferLevel level, size_t count,
                      VkCommandBuffer *buffers) const;

private:
  const PhysicalDevice &physicalDevice;
  const Device &device;

  VkCommandPool commandPool;

  VkCommandPool createCommandPool();
};
} // namespace Vulking
