#include "GPU.hpp"

Vulking::GPU::GPU(Instance instance, Surface surface)
    : physicalDevice(PhysicalDevice(instance, surface)), device(physicalDevice),
      graphicsQueue(device, getGraphicsQueueFamily()),
      presentQueue(device, getPresentQueueFamily()) {}

uint32_t Vulking::GPU::getGraphicsQueueFamily() const {
  assert(physicalDevice.queueFamilyIndices.isComplete());
  return physicalDevice.queueFamilyIndices.graphicsFamily.value();
}

uint32_t Vulking::GPU::getPresentQueueFamily() const {
  assert(physicalDevice.queueFamilyIndices.isComplete());
  return physicalDevice.queueFamilyIndices.presentFamily.value();
}
