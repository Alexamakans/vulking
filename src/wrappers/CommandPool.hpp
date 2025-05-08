#pragma once

#include <vulkan/vulkan_core.h>
class CommandPool {
public:
  CommandPool(PhysicalDevice physicalDevice) {}

private:
  VkCommandPool commandPool;
  VkCommandPool createCommandPool();
};
