#pragma once
#include "../common.hpp"
#include "Instance.hpp"
#include "Surface.hpp"
#include <optional>
#include <vector>

namespace Vulking {
class PhysicalDevice {
public:
  struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() const {
      return graphicsFamily.has_value() && presentFamily.has_value();
    }
  };

  const QueueFamilyIndices queueFamilyIndices{};

  PhysicalDevice() = default;
  PhysicalDevice(PhysicalDevice &) = default;
  PhysicalDevice(VkInstance instance, Surface surface);

  PhysicalDevice &operator=(const PhysicalDevice &other) {
    instance = other.instance;
    surface = other.surface;
    physicalDevice = other.physicalDevice;
    return *this;
  }
  operator VkPhysicalDevice() const;

  uint32_t findMemoryType(uint32_t typeFilter,
                          VkMemoryPropertyFlags properties) const;
  VkPhysicalDeviceProperties getProperties() const;
  VkPhysicalDeviceFeatures getFeatures() const;

private:
  Instance instance;
  Surface surface;
  VkPhysicalDevice physicalDevice{};

  QueueFamilyIndices getQueueFamilyIndices();
  VkPhysicalDevice getSuitablePhysicalDevice();
  bool isDeviceSuitable(VkPhysicalDevice dev) const;
};
} // namespace Vulking
