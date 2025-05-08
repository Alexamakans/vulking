#include "Device.hpp"
#include <cassert>
#include <set>

Vulking::Device::Device(PhysicalDevice physicalDevice)
    : physicalDevice(physicalDevice) {
  assert(physicalDevice.queueFamilyIndices.isComplete());
  createLogicalDevice(physicalDevice.queueFamilyIndices.graphicsFamily.value(), physicalDevice.queueFamilyIndices.presentFamily.value());
}

Vulking::Device::~Device() {
  if (device) {
    vkDestroyDevice(device, allocator);
  }
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
  if (ENABLE_SAMPLE_SHADING) {
    deviceFeatures.sampleRateShading = VK_TRUE;
  }

  VkDeviceCreateInfo createInfo{};
  createInfo.sType = STYPE(DEVICE_CREATE_INFO);
  createInfo.queueCreateInfoCount =
      static_cast<uint32_t>(queueCreateInfos.size());
  createInfo.pQueueCreateInfos = queueCreateInfos.data();
  createInfo.pEnabledFeatures = &deviceFeatures;
  createInfo.enabledExtensionCount = 0;
  createInfo.ppEnabledExtensionNames = nullptr;

  if (enableValidationLayers) {
    static const char *validationLayers[] = {"VK_LAYER_KHRONOS_validation"};
    createInfo.enabledLayerCount = 1;
    createInfo.ppEnabledLayerNames = validationLayers;
  } else {
    createInfo.enabledLayerCount = 0;
  }

  CHK(vkCreateDevice(physicalDevice, &createInfo, allocator, &device),
      "Failed to create logical device.");
}
