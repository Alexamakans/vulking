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
  static constexpr vk::BufferUsageFlags UNIFORM =
      vk::BufferUsageFlagBits::eUniformBuffer;
};

struct BufferMemory {
  static constexpr vk::MemoryPropertyFlags STAGING =
      vk::MemoryPropertyFlagBits::eHostVisible |
      vk::MemoryPropertyFlagBits::eHostCoherent;
  static constexpr vk::MemoryPropertyFlags FINAL =
      vk::MemoryPropertyFlagBits::eDeviceLocal;
  static constexpr vk::MemoryPropertyFlags UNIFORM =
      vk::MemoryPropertyFlagBits::eHostVisible |
      vk::MemoryPropertyFlagBits::eHostCoherent;
};
template <typename T> class Buffer {
public:
  Buffer() {}
  Buffer(const T *src, vk::DeviceSize size, vk::BufferUsageFlags usage,
         vk::MemoryPropertyFlags properties, const char *name = "unnamed");
  Buffer(const std::vector<T> &src, vk::BufferUsageFlags usage,
         vk::MemoryPropertyFlags properties, const char *name = "unnamed");
  Buffer(vk::DeviceSize size, vk::BufferUsageFlags usage,
         vk::MemoryPropertyFlags properties, const char *name = "unnamed");

  vk::Buffer getBuffer() const { return buffer.get(); }
  vk::DeviceMemory getMemory() const { return memory.get(); }
  vk::DeviceSize getSize() const { return size; }

  bool isMapped() const { return pData != nullptr; }
  void map();
  void mapTo(void **mapped);
  void set(const T *src, size_t size) const;
  void set(const std::vector<T> &src) const;
  void unmap();

  void copyTo(const Buffer &dst);

private:
  void init(vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties,
            const char *name = "unnamed");

  vk::DeviceSize size;
  vk::UniqueBuffer buffer;
  vk::UniqueDeviceMemory memory;
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
Buffer<T>::Buffer(const T *src, vk::DeviceSize size, vk::BufferUsageFlags usage,
                  vk::MemoryPropertyFlags properties, const char *name)
    : size(size) {
  assert(size != 0);
  init(usage, properties, name);
  map();
  set(src, size);
  unmap();
}

template <typename T> void Buffer<T>::map() { mapTo(&pData); }

template <typename T> void Buffer<T>::mapTo(void **mapped) {
  *mapped = Engine::device->mapMemory(memory.get(), 0, size);
  pData = *mapped;
}

template <typename T> void Buffer<T>::set(const T *src, size_t size) const {
  if (static_cast<VkDeviceSize>(size) > this->size) {
    throw std::runtime_error(
        std::format("size ({}) too large, must be <= {}", size, this->size));
  }
  assert(isMapped());
  memcpy(pData, src, size);
}

template <typename T> void Buffer<T>::set(const std::vector<T> &src) const {
  set(src.data(), sizeof(T) * src.size());
}

template <typename T> void Buffer<T>::unmap() {
  Engine::device->unmapMemory(memory.get());
  pData = nullptr;
}

template <typename T> void Buffer<T>::copyTo(const Buffer &dst) {
  assert(buffer);
  assert(dst.buffer);
  auto cmd = Engine::beginCommand();
  vk::BufferCopy copy{};
  copy.size = size;
  cmd.copyBuffer(buffer.get(), dst.buffer.get(), 1, &copy);
  cmd.end();
}

template <typename T>
void Buffer<T>::init(vk::BufferUsageFlags usage,
                     vk::MemoryPropertyFlags properties, const char *name) {
  vk::BufferCreateInfo info{};
  info.setSize(size);
  info.setUsage(usage);
  info.setSharingMode(vk::SharingMode::eExclusive);
  buffer = Engine::device->createBufferUnique(info);

  auto memoryRequirements =
      Engine::device->getBufferMemoryRequirements(buffer.get());

  vk::MemoryAllocateInfo allocInfo{};
  allocInfo.setAllocationSize(memoryRequirements.size);
  allocInfo.setMemoryTypeIndex(findMemoryType(
      Engine::physicalDevice, memoryRequirements.memoryTypeBits, properties));

  memory = Engine::device->allocateMemoryUnique(allocInfo, ALLOCATOR);

  NAME_OBJECT(Engine::device, buffer.get(), std::format("{}_buffer", name));
  NAME_OBJECT(Engine::device, memory.get(), std::format("{}_memory", name));
}
} // namespace Vulking
