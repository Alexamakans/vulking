#pragma once

#include "common.hpp"

class Device;
class Queue {
public:
  Queue(const Device device, uint32_t queueFamilyIndex);
  ~Queue();

  operator VkQueue() const;

private:
  Device device;
  VkQueue queue;
};
