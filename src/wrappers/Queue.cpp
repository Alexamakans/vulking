#include "Queue.hpp"

Queue::Queue(const Device &dev, uint32_t queueFamilyIndex) : device(dev) {
  vkGetDeviceQueue(device, queueFamilyIndex, 0, &queue);
}

Queue::~Queue() {
  // No explicit cleanup needed for VkQueue; managed by the VkDevice
}

Queue::operator VkQueue() const { return queue; };
