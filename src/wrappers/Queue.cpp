#include "Queue.hpp"
#include "Device.hpp"

Vulking::Queue::Queue(Device dev, uint32_t queueFamilyIndex) : device(dev) {
  vkGetDeviceQueue(device, queueFamilyIndex, 0, &queue);
}

Vulking::Queue::~Queue() {
  // No explicit cleanup needed for VkQueue; managed by the VkDevice
}

Vulking::Queue::operator VkQueue() const { return queue; };
