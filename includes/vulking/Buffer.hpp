#pragma once

#include "Common.hpp"
#include "Engine.hpp"

#include <cstring>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_enums.hpp>

namespace Vulking {
struct BufferUsage {
  static constexpr vk::BufferUsageFlags STAGING =
      vk::BufferUsageFlagBits::eTransferSrc;
  static constexpr vk::BufferUsageFlags FINAL =
      vk::BufferUsageFlagBits::eTransferDst;
  static constexpr vk::BufferUsageFlags UNIFORM =
      vk::BufferUsageFlagBits::eUniformBuffer;
  static constexpr vk::BufferUsageFlags FINAL_VERTEX_BUFFER =
      FINAL | vk::BufferUsageFlagBits::eVertexBuffer;
  static constexpr vk::BufferUsageFlags FINAL_INDEX_BUFFER =
      FINAL | vk::BufferUsageFlagBits::eIndexBuffer;
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
  Buffer() { LOG_DEBUG("buffer default constructor: " << _name); };
  Buffer(const Buffer &) = delete;
  Buffer &operator=(const Buffer &) = delete;
  Buffer(Buffer &&other) noexcept
      : buffer(std::move(other.buffer)), memory(std::move(other.memory)),
        _name(std::move(other._name)), size(other.size), pData(other.pData) {
    LOG_DEBUG("buffer move: " << _name);
  }
  Buffer &operator=(Buffer &&other) noexcept {
    if (this != &other) {
      LOG_DEBUG("buffer move assignment: " << _name << " <- " << other._name);
      if (isMapped()) {
        unmap();
      }
      buffer = std::move(other.buffer);
      memory = std::move(other.memory);
      _name = std::move(other._name);
      size = other.size;
      if (other.isMapped()) {
        pData = other.pData;
        other.pData = nullptr;
      }
    }
    return *this;
  };

  Buffer(const T *src, vk::DeviceSize size, vk::BufferUsageFlags usage,
         vk::MemoryPropertyFlags properties, const char *name = "unnamed");
  Buffer(const vk::ArrayProxy<T> &src, vk::BufferUsageFlags usage,
         vk::MemoryPropertyFlags properties, const char *name = "unnamed");
  Buffer(vk::DeviceSize size, vk::BufferUsageFlags usage,
         vk::MemoryPropertyFlags properties, const char *name = "unnamed");

  const vk::Buffer &getBuffer() const { return buffer.get(); }
  const vk::DeviceMemory &getMemory() const { return memory.get(); }
  const vk::DeviceSize &getSize() const { return size; }

  bool isMapped() const { return pData != nullptr; }
  void map();
  void mapTo(void **mapped);
  void set(const T *src, size_t size) const;
  void set(const vk::ArrayProxy<T> &src) const;
  void unmap();

  void copyTo(const Buffer &dst);

private:
  void init(vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties,
            const char *name = "unnamed");

  vk::DeviceSize size;
  vk::UniqueBuffer buffer;
  vk::UniqueDeviceMemory memory;
  void *pData = nullptr;
  std::string _name = "uninitialized";
};

template <typename T>
inline Buffer<T>::Buffer(vk::DeviceSize size, vk::BufferUsageFlags usage,
                         vk::MemoryPropertyFlags properties, const char *name)
    : size(size) {
  LOG_DEBUG("buffer 1 constructor: " << name);
  assert(size != 0);
  init(usage, properties, name);
}

template <typename T>
inline Buffer<T>::Buffer(const vk::ArrayProxy<T> &src,
                         vk::BufferUsageFlags usage,
                         vk::MemoryPropertyFlags properties, const char *name)
    : size(sizeof(T) * src.size()) {
  LOG_DEBUG("buffer 2 constructor: " << name);
  assert(size != 0);
  init(usage, properties, name);
  map();
  set(src.data(), size);
  unmap();
}

template <typename T>
Buffer<T>::Buffer(const T *src, vk::DeviceSize size, vk::BufferUsageFlags usage,
                  vk::MemoryPropertyFlags properties, const char *name)
    : size(size) {
  LOG_DEBUG("buffer 3 constructor: " << name);
  assert(size != 0);
  init(usage, properties, name);
  map();
  set(src, size);
  unmap();
}

template <typename T> void Buffer<T>::map() { mapTo(&pData); }

template <typename T> void Buffer<T>::mapTo(void **mapped) {
  LOG_DEBUG("\tmapped buffer: " << _name);
  assert(!isMapped());
  *mapped = Engine::ctx().device->mapMemory(memory.get(), 0, size);
  pData = *mapped;
}

template <typename T> void Buffer<T>::set(const T *src, size_t size) const {
  if (static_cast<VkDeviceSize>(size) > this->size) {
    throw std::runtime_error(
        std::format("size ({}) too large, must be <= {}", size, this->size));
  }
  LOG_DEBUG("\t\tset buffer: " << _name);
  assert(isMapped());
  memcpy(pData, src, size);
}

template <typename T> void Buffer<T>::set(const vk::ArrayProxy<T> &src) const {
  set(src.data(), sizeof(T) * src.size());
}

template <typename T> void Buffer<T>::unmap() {
  LOG_DEBUG("\tunmapped buffer: " << _name);
  assert(isMapped());
  Engine::ctx().device->unmapMemory(memory.get());
  pData = nullptr;
}

template <typename T> void Buffer<T>::copyTo(const Buffer &dst) {
  LOG_DEBUG("copy buffer " << _name << " -> ");
  assert(buffer);
  assert(dst.buffer);
  assert(size != 0);
  auto cmd = Engine::ctx().beginCommand(
      std::format("copyTo_command_{}", _name).c_str());
  cmd.copyBuffer(buffer.get(), dst.getBuffer(), vk::BufferCopy().setSize(size));
  Engine::ctx().endAndSubmitGraphicsCommand(std::move(cmd));
}

template <typename T>
void Buffer<T>::init(vk::BufferUsageFlags usage,
                     vk::MemoryPropertyFlags properties, const char *name) {
  _name = name;
  vk::BufferCreateInfo info{};
  info.setSize(size);
  info.setUsage(usage);
  info.setSharingMode(vk::SharingMode::eExclusive);
  buffer = Engine::ctx().device->createBufferUnique(info);

  auto memoryRequirements =
      Engine::ctx().device->getBufferMemoryRequirements(buffer.get());

  vk::MemoryAllocateInfo allocInfo{};
  allocInfo.setAllocationSize(memoryRequirements.size);
  allocInfo.setMemoryTypeIndex(findMemoryType(Engine::ctx().physicalDevice,
                                              memoryRequirements.memoryTypeBits,
                                              properties));

  memory = Engine::ctx().device->allocateMemoryUnique(allocInfo, ALLOCATOR);

  NAME_OBJECT(Engine::ctx().device, buffer.get(),
              std::format("{}_buffer", name));
  NAME_OBJECT(Engine::ctx().device, memory.get(),
              std::format("{}_memory", name));

  Engine::ctx().device->bindBufferMemory(buffer.get(), memory.get(), 0);
}
} // namespace Vulking
