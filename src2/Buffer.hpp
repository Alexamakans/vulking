#pragma once

#include "Common.hpp"
#include <cstring>

namespace Vulking {
class Buffer {
public:
  MOVE_ONLY(Buffer);

  Buffer(void *src, vk::DeviceSize size,
         vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties,
         const char *name = "unnamed");
  ~Buffer();

  bool isMapped() const;
  void map();
  void mapTo(void **mapped);
  void set(void *src, size_t size) const;
  void unmap();

  struct Usage {
    static constexpr vk::BufferUsageFlags STAGING =
        vk::BufferUsageFlagBits::eTransferSrc;
    static constexpr vk::BufferUsageFlags FINAL =
        vk::BufferUsageFlagBits::eTransferDst;
  };

  struct Memory {
    static constexpr vk::MemoryPropertyFlags STAGING =
        vk::MemoryPropertyFlagBits::eHostVisible |
        vk::MemoryPropertyFlagBits::eHostCoherent;
    static constexpr vk::MemoryPropertyFlags FINAL =
        vk::MemoryPropertyFlagBits::eDeviceLocal;
  };

private:
  vk::DeviceSize size;
  vk::Buffer buffer;
  vk::DeviceMemory memory;
  void *pData = nullptr;
};
} // namespace Vulking
