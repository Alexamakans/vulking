#pragma once

#include "../common.hpp"
#include "Device.hpp"

namespace Vulking {
class Queue {
public:
  Queue() = default;
  Queue(Queue&) = default;
  Queue(Device device, uint32_t queueFamilyIndex);
  ~Queue();

  Queue& operator=(const Queue& other) {
    device = other.device;
    queue = other.queue;
    return *this;
  }
  operator VkQueue() const;

private:
  Device device;
  VkQueue queue;
};
} // namespace Vulking
