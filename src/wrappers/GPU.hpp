#include "../common.hpp"

#include "Device.hpp"
#include "PhysicalDevice.hpp"
#include "Queue.hpp"

namespace Vulking {
class GPU {
public:
  GPU(PhysicalDevice physicalDevice, Device device);

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
