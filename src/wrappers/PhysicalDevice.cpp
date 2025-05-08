#include "PhysicalDevice.hpp"

Vulking::PhysicalDevice::PhysicalDevice(VkInstance instance, Surface surface)
    : instance(instance), surface(surface),
      physicalDevice(getSuitablePhysicalDevice()),
      queueFamilyIndices(getQueueFamilyIndices()) {}

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
    throw std::runtime_error("Failed to find GPUs with Vulkan support.");
  }

  std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
  vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices.data());

  for (const auto &physicalDevice : physicalDevices) {
    if (isDeviceSuitable(physicalDevice)) {
      return physicalDevice;
    }
  }

  throw std::runtime_error("Failed to find a suitable GPU.");
}

bool Vulking::PhysicalDevice::isDeviceSuitable(VkPhysicalDevice dev) const {
  // This should be extended based on required features, extensions, and queue
  // families.
  VkPhysicalDeviceProperties props;
  vkGetPhysicalDeviceProperties(dev, &props);

  return props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ||
         props.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
}

uint32_t Vulking::PhysicalDevice::findMemoryType(
    uint32_t typeFilter, VkMemoryPropertyFlags properties) const {
  VkPhysicalDeviceMemoryProperties memProperties;
  vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

  for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
    if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags &
                                    properties) == properties) {
      return i;
    }
  }

  throw std::runtime_error("Failed to find suitable memory type.");
}
