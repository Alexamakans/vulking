#pragma once
#include "common.hpp"
#include <optional>
#include <vector>

class PhysicalDevice {
public:
  explicit PhysicalDevice(VkInstance instance);
  operator VkPhysicalDevice() const;

  uint32_t findMemoryType(uint32_t typeFilter,
                          VkMemoryPropertyFlags properties) const;
  VkPhysicalDeviceProperties getProperties() const;
  VkPhysicalDeviceFeatures getFeatures() const;

private:
  VkPhysicalDevice device{};
  VkInstance instance;

  void pickPhysicalDevice();
  bool isDeviceSuitable(VkPhysicalDevice dev) const;
};
