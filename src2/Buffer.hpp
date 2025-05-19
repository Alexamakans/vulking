#pragma once

#include "Common.hpp"
#include "Device.hpp"
#include <cstring>

namespace Vulking {
class Buffer {
public:
  MOVE_ONLY(Buffer);

  Buffer(const Device &device, void *src, VkDeviceSize size,
         VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
         const char *name = "unnamed");
  ~Buffer();

  bool isMapped() const;
  void map();
  void mapTo(void **mapped);
  void set(void *src, size_t size) const;
  void unmap();

  struct Usage {
    static constexpr VkBufferUsageFlags STAGING =
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    static constexpr VkBufferUsageFlags FINAL =
        VK_BUFFER_USAGE_TRANSFER_DST_BIT;
  };

  struct Memory {
    static constexpr VkMemoryPropertyFlags STAGING =
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    static constexpr VkMemoryPropertyFlags FINAL =
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
  };

private:
  VkDevice device;

  VkDeviceSize size;
  VkBuffer buffer;
  VkDeviceMemory memory;
  void *pData = nullptr;
};
} // namespace Vulking
