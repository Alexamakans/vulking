#pragma once

#include "Common.hpp"
#include "Engine.hpp"

#include <cstring>
#include <vulkan/vulkan.hpp>

namespace Vulking {
struct BufferUsage {
  static constexpr vk::BufferUsageFlags STAGING =
      vk::BufferUsageFlagBits::eTransferSrc;
  static constexpr vk::BufferUsageFlags FINAL =
      vk::BufferUsageFlagBits::eTransferDst;
};

struct BufferMemory {
  static constexpr vk::MemoryPropertyFlags STAGING =
      vk::MemoryPropertyFlagBits::eHostVisible |
      vk::MemoryPropertyFlagBits::eHostCoherent;
  static constexpr vk::MemoryPropertyFlags FINAL =
      vk::MemoryPropertyFlagBits::eDeviceLocal;
};
template <typename T> class Buffer {
public:
  MOVE_ONLY(Buffer);

  Buffer() {}
  Buffer(void *src, vk::DeviceSize size, vk::BufferUsageFlags usage,
         vk::MemoryPropertyFlags properties, const char *name = "unnamed");
  Buffer(const std::vector<T> &src, vk::BufferUsageFlags usage,
         vk::MemoryPropertyFlags properties, const char *name = "unnamed");
  Buffer(vk::DeviceSize size, vk::BufferUsageFlags usage,
         vk::MemoryPropertyFlags properties, const char *name = "unnamed");
  ~Buffer();

  vk::Buffer get() const { return buffer; }
  vk::DeviceSize getSize() const { return size; }

  bool isMapped() const;
  void map();
  void mapTo(void **mapped);
  void set(void *src, size_t size) const;
  void unmap();

  void copyTo(const Buffer &dst);

private:
  void init(vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties,
            const char *name = "unnamed");

  vk::DeviceSize size;
  vk::Buffer buffer;
  vk::DeviceMemory memory;
  void *pData = nullptr;
};

template <typename T>
inline Buffer<T>::Buffer(vk::DeviceSize size, vk::BufferUsageFlags usage,
                         vk::MemoryPropertyFlags properties, const char *name)
    : size(size) {
  init(usage, properties, name);
}

template <typename T>
inline Buffer<T>::Buffer(const std::vector<T> &src, vk::BufferUsageFlags usage,
                         vk::MemoryPropertyFlags properties, const char *name)
    : size(sizeof(T) * src.size()) {
  init(usage, properties, name);
  set(src.data(), size);
}

template <typename T>
Buffer<T>::Buffer(void *src, vk::DeviceSize size, vk::BufferUsageFlags usage,
                  vk::MemoryPropertyFlags properties, const char *name)
    : size(size) {
  assert(size != 0);
  init(size, usage, properties, name);
  set(src, size);
}

template <typename T> Buffer<T>::~Buffer() {
  assert(buffer);
  Engine::device->destroyBuffer(buffer, ALLOCATOR);

  assert(memory);
  Engine::device->freeMemory(memory, ALLOCATOR);
}

template <typename T> void Buffer<T>::map() { mapTo(&pData); }

template <typename T> void Buffer<T>::mapTo(void **mapped) {
  *mapped = Engine::device->mapMemory(memory, 0, size);
  pData = *mapped;
}

template <typename T> void Buffer<T>::set(void *src, size_t size) const {
  if (static_cast<VkDeviceSize>(size) > this->size) {
    throw std::runtime_error(
        std::format("size ({}) too large, must be <= {}", size, this->size));
  }
  memcpy(pData, src, size);
}

template <typename T> void Buffer<T>::unmap() {
  Engine::device->unmapMemory(memory);
  pData = nullptr;
}

template <typename T> void Buffer<T>::copyTo(const Buffer &dst) {
  auto cmd = Engine::beginCommand();
  vk::BufferCopy copy{};
  copy.size = size;
  cmd.copyBuffer(buffer, dst.buffer, 1, &copy);
  cmd.end();
}

template <typename T>
void Buffer<T>::init(vk::BufferUsageFlags usage,
                     vk::MemoryPropertyFlags properties, const char *name) {
  vk::BufferCreateInfo info{};
  info.setSize(size);
  info.setUsage(usage);
  info.setSharingMode(vk::SharingMode::eExclusive);
  buffer = Engine::device->createBuffer(info, ALLOCATOR);

  auto memoryRequirements = Engine::device->getBufferMemoryRequirements(buffer);

  vk::MemoryAllocateInfo allocInfo{};
  allocInfo.setAllocationSize(memoryRequirements.size);
  allocInfo.setMemoryTypeIndex(findMemoryType(
      Engine::physicalDevice, memoryRequirements.memoryTypeBits, properties));

  memory = Engine::device->allocateMemory(allocInfo, ALLOCATOR);

  NAME_OBJECT(Engine::device, buffer, std::format("{}_buffer", name));
  NAME_OBJECT(Engine::device, memory, std::format("{}_memory", name));
}
} // namespace Vulking
