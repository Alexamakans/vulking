#include <set>

#include "../common.hpp"
#include "../engine.hpp"
#include "gpu.hpp"

GPU::GPU(VkPhysicalDevice device)
    : device(device), queueFamilyIndices(getQueueFamilyIndices()),
      swapchainSupportDetails(getSwapchainSupportDetails()),
      msaaSamples(getMaxUsableSampleCount()) {}

GPU::~GPU() {};

int GPU::rateSuitability() {
  VkPhysicalDeviceProperties deviceProperties;
  VkPhysicalDeviceFeatures deviceFeatures;
  vkGetPhysicalDeviceProperties(device, &deviceProperties);
  vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

  int score = 0;

  if (!deviceFeatures.geometryShader) {
    // Application can't function without geometry shaders
    return 0;
  }

  if (!deviceFeatures.samplerAnisotropy) {
    return 0;
  }

  if (!queueFamilyIndices.isComplete()) {
    return 0;
  }

  if (!checkDeviceExtensionSupport()) {
    return 0;
  }

  bool swapChainAdequate = !swapchainSupportDetails.formats.empty() &&
                           !swapchainSupportDetails.presentModes.empty();

  // Discrete GPUs have a significant performance advantage
  if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
    score += 1000;
  }

  // Maximum possible size of textures affects graphics quality
  score += deviceProperties.limits.maxImageDimension2D;

  return score;
}

GPU::QueueFamilyIndices GPU::getQueueFamilyIndices() {
  QueueFamilyIndices indices;

  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
  std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,
                                           queueFamilies.data());

  int i = 0;
  for (const auto &queueFamily : queueFamilies) {
    if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      indices.graphicsFamily = i;
    }

    VkBool32 presentSupport = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(device, i, Engine::surface,
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

bool GPU::checkDeviceExtensionSupport() {
  uint32_t extensionCount;
  CHK(vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
                                           nullptr),
      "failed to get device extension properties count")

  std::vector<VkExtensionProperties> availableExtensions(extensionCount);
  CHK(vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
                                           availableExtensions.data()),
      "failed to enumerate device extension properties")

  std::set<std::string> requiredExtensions(deviceExtensions.begin(),
                                           deviceExtensions.end());

  for (const auto &extension : availableExtensions) {
    requiredExtensions.erase(extension.extensionName);
  }

  return requiredExtensions.empty();
}

GPU::SwapchainSupportDetails GPU::getSwapchainSupportDetails() {
  SwapchainSupportDetails details;
  CHK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, Engine::surface,
                                                &details.capabilities),
      "failed to get physical device Engine::surface "
      "capabilities")

  uint32_t formatCount;
  CHK(vkGetPhysicalDeviceSurfaceFormatsKHR(device, Engine::surface,
                                           &formatCount, nullptr),
      "failed to get physical device Engine::surface formats "
      "count");

  if (formatCount != 0) {
    details.formats.resize(formatCount);
    CHK(vkGetPhysicalDeviceSurfaceFormatsKHR(
            device, Engine::surface, &formatCount, details.formats.data()),
        "failed to get physical device Engine::surface "
        "formats");
  }

  uint32_t presentModeCount;
  CHK(vkGetPhysicalDeviceSurfacePresentModesKHR(device, Engine::surface,
                                                &presentModeCount, nullptr),
      "failed to get physical device Engine::surface present "
      "modes "
      "count");
  if (presentModeCount != 0) {
    details.presentModes.resize(presentModeCount);
    CHK(vkGetPhysicalDeviceSurfacePresentModesKHR(device, Engine::surface,
                                                  &presentModeCount,
                                                  details.presentModes.data()),
        "failed to get physical device Engine::surface present "
        "modes");
  }

  return swapchainSupportDetails;
}

VkSampleCountFlagBits GPU::getMaxUsableSampleCount() {
  VkPhysicalDeviceProperties properties;
  vkGetPhysicalDeviceProperties(device, &properties);

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

uint32_t GPU::findMemoryType(uint32_t typeFilter,
                             VkMemoryPropertyFlags properties) {
  VkPhysicalDeviceMemoryProperties memoryProperties;
  vkGetPhysicalDeviceMemoryProperties(device, &memoryProperties);

  for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
    if ((typeFilter & (1 << i)) &&
        (memoryProperties.memoryTypes[i].propertyFlags & properties) ==
            properties) {
      return i;
    }
  }

  throw std::runtime_error("failed to find suitable memory type");
}

VkFormat GPU::findSupportedFormat(const std::vector<VkFormat> &candidates,
                                  VkImageTiling tiling,
                                  VkFormatFeatureFlags features) {
  for (VkFormat format : candidates) {
    VkFormatProperties properties;
    vkGetPhysicalDeviceFormatProperties(device, format, &properties);

    if (tiling == VK_IMAGE_TILING_LINEAR &&
        (properties.linearTilingFeatures & features) == features) {
      return format;
    }

    if (tiling == VK_IMAGE_TILING_OPTIMAL &&
        (properties.optimalTilingFeatures & features) == features) {
      return format;
    }
  }

  throw std::runtime_error("failed to find supported format");
}
