#pragma once

#include "gpu.hpp"
#include <vector>
#include <vulkan/vulkan_core.h>

class LogicalDevice {
public:
  LogicalDevice(const GPU &gpu, VkSurfaceKHR surface);
  ~LogicalDevice();

  operator VkDevice() const { return device; };

  VkQueue getGraphicsQueue() const { return graphicsQueue; }
  VkQueue getPresentQueue() const { return presentQueue; }

  VkCommandPool createCommandPool(VkCommandPoolCreateFlags flags = 0) const;

private:
  VkDevice device;
  VkQueue graphicsQueue;
  VkQueue presentQueue;
  const GPU &gpu;

  void createLogicalDevice(VkSurfaceKHR surface);
};
