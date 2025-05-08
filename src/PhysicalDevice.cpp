#include "PhysicalDevice.hpp"

PhysicalDevice::PhysicalDevice(VkInstance inst) : instance(inst) {
  pickPhysicalDevice();
}

PhysicalDevice::operator VkPhysicalDevice() const { return device; }

VkPhysicalDeviceProperties PhysicalDevice::getProperties() const {
  VkPhysicalDeviceProperties props;
  vkGetPhysicalDeviceProperties(device, &props);
  return props;
}

VkPhysicalDeviceFeatures PhysicalDevice::getFeatures() const {
  VkPhysicalDeviceFeatures features;
  vkGetPhysicalDeviceFeatures(device, &features);
  return features;
}

void PhysicalDevice::pickPhysicalDevice() {
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
  if (deviceCount == 0) {
    throw std::runtime_error("Failed to find GPUs with Vulkan support.");
  }

  std::vector<VkPhysicalDevice> devices(deviceCount);
  vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

  for (const auto &dev : devices) {
    if (isDeviceSuitable(dev)) {
      device = dev;
      return;
    }
  }

  throw std::runtime_error("Failed to find a suitable GPU.");
}

bool PhysicalDevice::isDeviceSuitable(VkPhysicalDevice dev) const {
  // This should be extended based on required features, extensions, and queue
  // families.
  VkPhysicalDeviceProperties props;
  vkGetPhysicalDeviceProperties(dev, &props);

  return props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ||
         props.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
}

uint32_t
PhysicalDevice::findMemoryType(uint32_t typeFilter,
                               VkMemoryPropertyFlags properties) const {
  VkPhysicalDeviceMemoryProperties memProperties;
  vkGetPhysicalDeviceMemoryProperties(device, &memProperties);

  for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
    if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags &
                                    properties) == properties) {
      return i;
    }
  }

  throw std::runtime_error("Failed to find suitable memory type.");
}
