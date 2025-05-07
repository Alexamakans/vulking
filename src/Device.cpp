#include "Device.hpp"
#include <set>

Device::Device(const PhysicalDevice &physDev)
    : physicalDevice(physDev) {
  findQueueFamilies();
  createLogicalDevice();
}

Device::~Device() {
  if (device) {
    vkDestroyDevice(device, allocator);
  }
}

Device::operator VkDevice() const { return device; }
Device::operator VkPhysicalDevice() const { return physicalDevice; };

VkQueue Device::getGraphicsQueue() const { return graphicsQueue; }

VkQueue Device::getPresentQueue() const { return presentQueue; }

uint32_t Device::getGraphicsQueueFamily() const { return graphicsQueueFamily; }

uint32_t Device::getPresentQueueFamily() const { return presentQueueFamily; }

void Device::findQueueFamilies() {
  uint32_t count = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, nullptr);
  std::vector<VkQueueFamilyProperties> families(count);
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count,
                                           families.data());

  for (uint32_t i = 0; i < count; ++i) {
    if (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      graphicsQueueFamily = i;
    }

    // Assume presentQueueFamily = graphicsQueueFamily unless overridden later
    presentQueueFamily = graphicsQueueFamily;

    if (graphicsQueueFamily != UINT32_MAX) {
      break;
    }
  }

  if (graphicsQueueFamily == UINT32_MAX) {
    throw std::runtime_error("Failed to find graphics queue family.");
  }
}

void Device::createLogicalDevice() {
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

  vkGetDeviceQueue(device, graphicsQueueFamily, 0, &graphicsQueue);
  vkGetDeviceQueue(device, presentQueueFamily, 0, &presentQueue);
}
