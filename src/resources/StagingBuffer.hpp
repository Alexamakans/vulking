#pragma once

#include "../common.hpp"
#include "../helpers/VulkingUtil.hpp"
#include "../wrappers/Device.hpp"
#include <vulkan/vulkan_core.h>

namespace Vulking {
class StagingBuffer {
public:
  StagingBuffer(const PhysicalDevice &physicalDevice, const Device &device,
                void *data, VkDeviceSize size, const char* name = "unnamed");
  ~StagingBuffer();

  operator VkBuffer() const;

private:
  const Device &device;

  void *dst;
  VkBuffer buffer;
  VkDeviceMemory memory;
};
} // namespace Vulking
