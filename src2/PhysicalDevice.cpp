#include "PhysicalDevice.hpp"

Vulking::PhysicalDevice::PhysicalDevice(VkPhysicalDevice physicalDevice,
                                        VkSurfaceKHR surface)
    : physicalDevice(physicalDevice) {
  init(surface);
}

Vulking::PhysicalDevice::operator VkPhysicalDevice() const {
  return physicalDevice;
}

VkFormat Vulking::PhysicalDevice::getColorFormat() const { return colorFormat; }

VkFormat Vulking::PhysicalDevice::getDepthFormat() const { return depthFormat; }

VkExtent2D Vulking::PhysicalDevice::getExtent() const { return extent; }

VkPresentModeKHR Vulking::PhysicalDevice::getPresentMode() const {
  return presentMode;
}

VkSampleCountFlagBits Vulking::PhysicalDevice::getMsaaSamples() const {
  return msaaSamples;
}

const VkPhysicalDeviceProperties &
Vulking::PhysicalDevice::getProperties() const {
  return properties;
}

VkFormat Vulking::PhysicalDevice::findSupportedFormat(
    const std::vector<VkFormat> &candidates, VkImageTiling tiling,
    VkFormatFeatureFlags features) const {
  for (VkFormat format : candidates) {
    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

    if (tiling == VK_IMAGE_TILING_LINEAR &&
        (props.linearTilingFeatures & features) == features) {
      return format;
    } else if (tiling == VK_IMAGE_TILING_OPTIMAL &&
               (props.optimalTilingFeatures & features) == features) {
      return format;
    }
  }

  throw std::runtime_error("failed to find supported format");
}

void Vulking::PhysicalDevice::init(VkSurfaceKHR surface) {
  vkGetPhysicalDeviceProperties(physicalDevice, &properties);

  msaaSamples = getMaxUsableMsaaSamples();

  queueFamilyIndices = getQueueFamilyIndices(surface);

  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface,
                                            &capabilities);

  uint32_t formatCount = 0;
  vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount,
                                       nullptr);
  std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
  vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount,
                                       surfaceFormats.data());

  VkSurfaceFormatKHR surfaceFormat = chooseSurfaceFormat(surfaceFormats);
  colorFormat = surfaceFormat.format;
  depthFormat = findDepthFormat();
  extent = capabilities.currentExtent;

  // Choose present mode (FIFO is guaranteed to be supported)
  presentMode = VK_PRESENT_MODE_FIFO_KHR;
}

VkSurfaceFormatKHR Vulking::PhysicalDevice::chooseSurfaceFormat(
    const std::vector<VkSurfaceFormatKHR> &availableFormats) {
  for (const auto &availableFormat : availableFormats) {
    if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
        availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      return availableFormat;
    }
  }

  return availableFormats[0];
}

Vulking::PhysicalDevice::QueueFamilyIndices
Vulking::PhysicalDevice::getQueueFamilyIndices(VkSurfaceKHR surface) {
  QueueFamilyIndices indices;

  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount,
                                           nullptr);
  std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount,
                                           queueFamilies.data());

  int i = 0;
  for (const auto &queueFamily : queueFamilies) {
    if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      indices.graphicsFamily = i;
    }

    VkBool32 presentSupport = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface,
                                         &presentSupport);
    if (presentSupport) {
      indices.presentFamily = i;
    }

    if (indices.isComplete()) {
      break;
    }
    i++;
  }

  return indices;
}

VkSampleCountFlagBits Vulking::PhysicalDevice::getMaxUsableMsaaSamples() const {
  VkSampleCountFlags counts = properties.limits.framebufferColorSampleCounts &
                              properties.limits.framebufferDepthSampleCounts;
  if (counts & VK_SAMPLE_COUNT_64_BIT) {
    return VK_SAMPLE_COUNT_64_BIT;
  }
  if (counts & VK_SAMPLE_COUNT_32_BIT) {
    return VK_SAMPLE_COUNT_32_BIT;
  }
  if (counts & VK_SAMPLE_COUNT_16_BIT) {
    return VK_SAMPLE_COUNT_16_BIT;
  }
  if (counts & VK_SAMPLE_COUNT_8_BIT) {
    return VK_SAMPLE_COUNT_8_BIT;
  }
  if (counts & VK_SAMPLE_COUNT_4_BIT) {
    return VK_SAMPLE_COUNT_4_BIT;
  }
  if (counts & VK_SAMPLE_COUNT_2_BIT) {
    return VK_SAMPLE_COUNT_2_BIT;
  }

  return VK_SAMPLE_COUNT_1_BIT;
}

VkFormat Vulking::PhysicalDevice::findDepthFormat() {
  return findSupportedFormat(
      {
          VK_FORMAT_D32_SFLOAT,
          VK_FORMAT_D32_SFLOAT_S8_UINT,
          VK_FORMAT_D24_UNORM_S8_UINT,
      },
      VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}
