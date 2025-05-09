#pragma once

#include "../common.hpp"
#include "Device.hpp"

namespace Vulking {
class Queue {
public:
  Queue(const Device &device, uint32_t queueFamilyIndex);

  operator VkQueue() const;

private:
  const Device& device;
  VkQueue queue;
};
} // namespace Vulking
