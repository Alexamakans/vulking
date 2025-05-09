#pragma once
#include "../common.hpp"
#include "Instance.hpp"
#include "Surface.hpp"
#include <optional>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace Vulking {
class PhysicalDevice {
public:
  struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() const {
      return graphicsFamily.has_value() && presentFamily.has_value();
    }

    QueueFamilyIndices &operator=(const QueueFamilyIndices &other) {
      graphicsFamily = other.graphicsFamily;
      presentFamily = other.presentFamily;
      return *this;
    }
  };
  QueueFamilyIndices queueFamilyIndices{};

  PhysicalDevice(const Instance &instance, const Surface &surface);
  operator VkPhysicalDevice() const;
  void init();

  uint32_t findMemoryType(uint32_t typeFilter,
                          VkMemoryPropertyFlags properties) const;
  VkPhysicalDeviceProperties getProperties() const;
  VkPhysicalDeviceFeatures getFeatures() const;
  VkFormat getFormat() const;
  VkExtent2D getExtent() const;
  const VkSurfaceCapabilitiesKHR &getCapabilities() const;
  VkPresentModeKHR getPresentMode() const;
  VkSampleCountFlagBits getMsaaSamples() const;

private:
  const Instance &instance;
  const Surface &surface;
  VkPhysicalDevice physicalDevice{};

  VkSurfaceCapabilitiesKHR capabilities;
  VkFormat format;
  VkExtent2D extent;
  VkPresentModeKHR presentMode;
  VkSampleCountFlagBits msaaSamples;

  QueueFamilyIndices getQueueFamilyIndices();
  VkPhysicalDevice getSuitablePhysicalDevice();
  bool isDeviceSuitable(VkPhysicalDevice dev) const;
  VkSampleCountFlagBits getMaxUsableMsaaSamples() const;
};
} // namespace Vulking
