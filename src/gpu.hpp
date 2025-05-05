#pragma once

#include <optional>
#include <set>
#include <vector>
#include <vulkan/vulkan_core.h>

#include "common.hpp"

const std::vector<const char *> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME};

class GPU {
public:
  struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() const {
      return graphicsFamily.has_value() && presentFamily.has_value();
    }
  };

  struct SwapchainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
  };

  const QueueFamilyIndices queueFamilyIndices;
  const SwapchainSupportDetails swapchainSupportDetails;
  const VkSampleCountFlagBits maxSamples;

  GPU(VkPhysicalDevice device, VkSurfaceKHR surface);

  int rateSuitability();

private:
  VkPhysicalDevice device;
  VkSurfaceKHR surface;
  
  QueueFamilyIndices getQueueFamilyIndices();
  bool checkDeviceExtensionSupport();
  SwapchainSupportDetails getSwapchainSupportDetails();
  VkSampleCountFlagBits getMaxUsableSampleCount();
};
