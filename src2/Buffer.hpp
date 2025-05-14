#pragma once

#include "Common.hpp"
#include "Device.hpp"
#include <cstring>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

namespace Vulking {
class Buffer {
public:
  Buffer(Device device, void *src, VkDeviceSize size, VkBufferUsageFlags usage,
         VkMemoryPropertyFlags properties, const char *name = "unnamed");

  bool isMapped() const;
  void map();
  void set(void *src, size_t size) const;
  void unmap();

private:
  const VkDevice device;
  const VkDeviceSize size;
  VkBuffer buffer;
  VkDeviceMemory memory;
  void *data = nullptr;
};
} // namespace Vulking
