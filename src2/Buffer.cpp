#include "Buffer.hpp"

#include "Common.hpp"
#include "Engine.hpp"

Vulking::Buffer::Buffer(void *src, vk::DeviceSize size,
                        vk::BufferUsageFlags usage,
                        vk::MemoryPropertyFlags properties, const char *name)
    : size(size) {
  assert(size != 0);

  vk::BufferCreateInfo info{};
  info.setSize(size);
  info.setUsage(usage);
  info.setSharingMode(vk::SharingMode::eExclusive);
  auto buffer = Engine::device->createBuffer(info, ALLOCATOR);

  auto memoryRequirements = Engine::device->getBufferMemoryRequirements(buffer);

  vk::MemoryAllocateInfo allocInfo{};
  allocInfo.setAllocationSize(memoryRequirements.size);
  allocInfo.setMemoryTypeIndex(findMemoryType(
      Engine::physicalDevice, memoryRequirements.memoryTypeBits, properties));

  auto memory = Engine::device->allocateMemory(allocInfo, ALLOCATOR);

  NAME_OBJECT(Engine::device, buffer, std::format("{}_buffer", name));
  NAME_OBJECT(Engine::device, memory, std::format("{}_memory", name));
}

Vulking::Buffer::~Buffer() {
  assert(buffer);
  Engine::device->destroyBuffer(buffer, ALLOCATOR);

  assert(memory);
  Engine::device->freeMemory(memory, ALLOCATOR);
}

void Vulking::Buffer::map() { mapTo(&pData); }

void Vulking::Buffer::mapTo(void **mapped) {
  *mapped = Engine::device->mapMemory(memory, 0, size);
  pData = *mapped;
}

void Vulking::Buffer::set(void *src, size_t size) const {
  if (static_cast<VkDeviceSize>(size) > this->size) {
    throw std::runtime_error(
        std::format("size ({}) too large, must be <= {}", size, this->size));
  }
  memcpy(pData, src, size);
}

void Vulking::Buffer::unmap() {
  Engine::device->unmapMemory(memory);
  pData = nullptr;
}
