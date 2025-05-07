#pragma once
#include "PhysicalDevice.hpp"
#include "Queue.hpp"
#include "common.hpp"
#include <optional>
#include <vector>

class Device {
public:
  Device(PhysicalDevice physicalDevice);
  ~Device();

  operator VkDevice() const;
  operator VkPhysicalDevice() const;
  Queue getGraphicsQueue() const;
  Queue getPresentQueue() const;
  uint32_t getGraphicsQueueFamily() const;
  uint32_t getPresentQueueFamily() const;

private:
  VkDevice device{};
  Queue graphicsQueue;
  Queue presentQueue;
  uint32_t graphicsQueueFamily{};
  uint32_t presentQueueFamily{};
  PhysicalDevice physicalDevice;

  void createLogicalDevice();
  void findQueueFamilies();
};
