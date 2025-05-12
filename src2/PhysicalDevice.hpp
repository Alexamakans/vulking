#pragma once

#include "Common.hpp"

#include <cstdint>
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

    QueueFamilyIndices &operator=(const QueueFamilyIndices &other) {
      graphicsFamily = other.graphicsFamily;
      presentFamily = other.presentFamily;
      return *this;
    }
  };
  QueueFamilyIndices queueFamilyIndices{};
  PhysicalDevice() = default;
  PhysicalDevice(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

  operator VkPhysicalDevice() const;

  VkFormat getColorFormat() const;
  VkFormat getDepthFormat() const;
  VkExtent2D getExtent() const;
  VkPresentModeKHR getPresentMode() const;
  VkSampleCountFlagBits getMsaaSamples() const;

  VkFormat findSupportedFormat(const std::vector<VkFormat> &candidates,
                               VkImageTiling tiling,
                               VkFormatFeatureFlags features) const;

  uint32_t findMemoryType(uint32_t typeFilter,
                          VkMemoryPropertyFlags properties) const {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
      if ((typeFilter & (1 << i)) &&
          (memProperties.memoryTypes[i].propertyFlags & properties) ==
              properties) {
        return i;
      }
    }

    throw std::runtime_error("failed to find suitable memory type.");
  }

private:
  VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

  VkSurfaceCapabilitiesKHR capabilities;

  VkFormat colorFormat;
  VkFormat depthFormat;
  VkExtent2D extent;
  // FIFO is guaranteed to be supported
  VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
  VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;

  void init(VkSurfaceKHR surface);

  VkSurfaceFormatKHR
  chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);

  QueueFamilyIndices getQueueFamilyIndices(VkSurfaceKHR surface);
  VkSampleCountFlagBits getMaxUsableMsaaSamples() const;
  VkFormat findDepthFormat();
};
} // namespace Vulking
