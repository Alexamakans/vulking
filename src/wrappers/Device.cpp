#include "Device.hpp"
#include <cassert>
#include <set>

const std::vector<const char *> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME};

Vulking::Device::Device(const PhysicalDevice &physicalDevice)
    : physicalDevice(physicalDevice) {
  assert(physicalDevice.queueFamilyIndices.isComplete());
  createLogicalDevice(physicalDevice.queueFamilyIndices.graphicsFamily.value(),
                      physicalDevice.queueFamilyIndices.presentFamily.value());
}

Vulking::Device::operator VkDevice() const { return device; }

void Vulking::Device::createLogicalDevice(uint32_t graphicsQueueFamily,
                                          uint32_t presentQueueFamily) {
  float queuePriority = 1.0f;

  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
  std::set<uint32_t> uniqueFamilies = {graphicsQueueFamily, presentQueueFamily};

  for (uint32_t queueFamily : uniqueFamilies) {
    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = STYPE(DEVICE_QUEUE_CREATE_INFO);
    queueCreateInfo.queueFamilyIndex = queueFamily;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;
    queueCreateInfos.push_back(queueCreateInfo);
  }

  VkPhysicalDeviceFeatures deviceFeatures{};
  deviceFeatures.samplerAnisotropy = VK_TRUE;
  if (ENABLE_SAMPLE_SHADING) {
    deviceFeatures.sampleRateShading = VK_TRUE;
  }

  VkDeviceCreateInfo createInfo{};
  createInfo.sType = STYPE(DEVICE_CREATE_INFO);
  createInfo.queueCreateInfoCount =
      static_cast<uint32_t>(queueCreateInfos.size());
  createInfo.pQueueCreateInfos = queueCreateInfos.data();
  createInfo.pEnabledFeatures = &deviceFeatures;
  createInfo.enabledExtensionCount =
      static_cast<uint32_t>(deviceExtensions.size());
  createInfo.ppEnabledExtensionNames = deviceExtensions.data();

  CHK(vkCreateDevice(physicalDevice, &createInfo, allocator, &device),
      "failed to create logical device.");
}
