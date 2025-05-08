#pragma once

#include "../common.hpp"

#include "../wrappers/Device.hpp"
#include "../wrappers/PhysicalDevice.hpp"
#include "../wrappers/Queue.hpp"

namespace Vulking {
class GPU {
public:
  GPU() = default;
  GPU(PhysicalDevice physicalDevice, Device device);
  GPU(GPU&) = default;

  operator VkPhysicalDevice() const { return physicalDevice; }
  operator VkDevice() const { return device; }

  uint32_t getGraphicsQueueFamily() const;
  uint32_t getPresentQueueFamily() const;

private:
  PhysicalDevice physicalDevice;
  Device device;

  Queue graphicsQueue;
  Queue presentQueue;

  uint32_t graphicsQueueFamily{};
  uint32_t presentQueueFamily{};
};
} // namespace Vulking
