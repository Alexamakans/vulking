#pragma once

#include "../common.hpp"

#include "../wrappers/Device.hpp"
#include "../wrappers/PhysicalDevice.hpp"
#include "../wrappers/Queue.hpp"

namespace Vulking {
class GPU {
public:
  GPU() {};
  GPU(GPU &gpu) {
    gpu.physicalDevice = physicalDevice;
    gpu.device = device;
    gpu.graphicsQueue = graphicsQueue;
    gpu.presentQueue = presentQueue;
    gpu.graphicsQueueFamily = graphicsQueueFamily;
    gpu.presentQueueFamily = presentQueueFamily;
  };
  GPU(Instance instance, Surface surface);

  operator VkPhysicalDevice() const { return physicalDevice; }
  operator VkDevice() const { return device; }

  uint32_t getGraphicsQueueFamily() const;
  uint32_t getPresentQueueFamily() const;

private:
  PhysicalDevice physicalDevice{};
  Device device{};

  Queue graphicsQueue{};
  Queue presentQueue{};

  uint32_t graphicsQueueFamily{};
  uint32_t presentQueueFamily{};
};
} // namespace Vulking
