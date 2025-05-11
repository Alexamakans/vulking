#include "PhysicalDevice.hpp"
#include "../helpers/VulkingUtil.hpp"
#include <vulkan/vulkan_core.h>

Vulking::PhysicalDevice::PhysicalDevice(const Instance &instance,
                                        const Surface &surface)
    : instance(instance), surface(surface),
      physicalDevice(getSuitablePhysicalDevice()) {
  init();
}

Vulking::PhysicalDevice::operator VkPhysicalDevice() const {
  return physicalDevice;
}

VkPhysicalDeviceProperties Vulking::PhysicalDevice::getProperties() const {
  VkPhysicalDeviceProperties props;
  vkGetPhysicalDeviceProperties(physicalDevice, &props);
  return props;
}

VkPhysicalDeviceFeatures Vulking::PhysicalDevice::getFeatures() const {
  VkPhysicalDeviceFeatures features;
  vkGetPhysicalDeviceFeatures(physicalDevice, &features);
  return features;
}

Vulking::PhysicalDevice::QueueFamilyIndices
Vulking::PhysicalDevice::getQueueFamilyIndices() {
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

VkPhysicalDevice Vulking::PhysicalDevice::getSuitablePhysicalDevice() {
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
  if (deviceCount == 0) {
    throw std::runtime_error("failed to find GPUs with Vulkan support.");
  }

  std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
  vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices.data());

  for (const auto &physicalDevice : physicalDevices) {
    if (isDeviceSuitable(physicalDevice)) {
      return physicalDevice;
    }
  }

  throw std::runtime_error("failed to find a suitable GPU.");
}

bool Vulking::PhysicalDevice::isDeviceSuitable(VkPhysicalDevice dev) const {
  // This should be extended based on required features, extensions, and queue
  // families.
  VkPhysicalDeviceProperties props;
  vkGetPhysicalDeviceProperties(dev, &props);

  return props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ||
         props.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
}

void Vulking::PhysicalDevice::init() {
  msaaSamples = getMaxUsableMsaaSamples();

  queueFamilyIndices = getQueueFamilyIndices();

  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface,
                                            &capabilities);
  std::cout << "got min image count: " << capabilities.minImageCount
            << std::endl;

  uint32_t formatCount = 0;
  vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount,
                                       nullptr);
  std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
  vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount,
                                       surfaceFormats.data());

  VkSurfaceFormatKHR surfaceFormat = surfaceFormats[0];
  format = surfaceFormat.format;
  depthFormat = VulkingUtil::findDepthFormat(physicalDevice);
  extent = capabilities.currentExtent;

  // Choose present mode (FIFO is guaranteed to be supported)
  presentMode = VK_PRESENT_MODE_FIFO_KHR;
}

VkFormat Vulking::PhysicalDevice::getFormat() const { return format; }
VkFormat Vulking::PhysicalDevice::getDepthFormat() const { return depthFormat; }
VkExtent2D Vulking::PhysicalDevice::getExtent() const { return extent; }

const VkSurfaceCapabilitiesKHR &
Vulking::PhysicalDevice::getCapabilities() const {
  return capabilities;
}

VkPresentModeKHR Vulking::PhysicalDevice::getPresentMode() const {
  return presentMode;
}
VkSampleCountFlagBits Vulking::PhysicalDevice::getMsaaSamples() const {
  return msaaSamples;
}

VkSampleCountFlagBits Vulking::PhysicalDevice::getMaxUsableMsaaSamples() const {
  VkPhysicalDeviceProperties physicalDeviceProperties;
  vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

  VkSampleCountFlags counts =
      physicalDeviceProperties.limits.framebufferColorSampleCounts &
      physicalDeviceProperties.limits.framebufferDepthSampleCounts;
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
