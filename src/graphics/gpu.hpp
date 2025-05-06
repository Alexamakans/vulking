#pragma once

#include <optional>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_core.h>

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
  const VkSampleCountFlagBits msaaSamples;

  GPU(VkPhysicalDevice device);
  ~GPU();

  operator VkPhysicalDevice() const { return device; }

  int rateSuitability();
  uint32_t findMemoryType(uint32_t typeFilter,
                          VkMemoryPropertyFlags properties);

  VkFormat findSupportedFormat(const std::vector<VkFormat> &candidates,
                               VkImageTiling tiling,
                               VkFormatFeatureFlags features);

private:
  const VkPhysicalDevice device;

  QueueFamilyIndices getQueueFamilyIndices();
  bool checkDeviceExtensionSupport();
  SwapchainSupportDetails getSwapchainSupportDetails();
  VkSampleCountFlagBits getMaxUsableSampleCount();
};
