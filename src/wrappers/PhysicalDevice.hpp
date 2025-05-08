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
  PhysicalDevice(PhysicalDevice&) = default;
  explicit PhysicalDevice(VkInstance instance, Surface surface);
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
