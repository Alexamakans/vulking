#include "Queue.hpp"
#include "Device.hpp"

Vulking::Queue::Queue(const Device &device, uint32_t queueFamilyIndex)
    : device(device) {
  vkGetDeviceQueue(device, queueFamilyIndex, 0, &queue);
}

Vulking::Queue::operator VkQueue() const { return queue; };
