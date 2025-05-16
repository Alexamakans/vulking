#include "Buffer.hpp"
#include <vulkan/vulkan_core.h>

Vulking::Buffer::Buffer(const Device &device, void *src, VkDeviceSize size,
                        VkBufferUsageFlags usage,
                        VkMemoryPropertyFlags properties, const char *name)
    : device(device), size(size) {
  assert(device != VK_NULL_HANDLE);
  assert(size != 0);

  VkBufferCreateInfo info{};
  info.sType = STYPE(BUFFER_CREATE_INFO);
  info.size = size;
  info.usage = usage;
  info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  CHK(vkCreateBuffer(device, &info, ALLOCATOR, &buffer),
      std::format("failed to create buffer '{}_buffer'", name));

  VkMemoryRequirements memoryRequirements{};
  vkGetBufferMemoryRequirements(device, buffer, &memoryRequirements);

  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = STYPE(MEMORY_ALLOCATE_INFO);
  allocInfo.allocationSize = memoryRequirements.size;
  allocInfo.memoryTypeIndex = device.getPhysical().findMemoryType(
      memoryRequirements.memoryTypeBits, properties);

  CHK(vkAllocateMemory(device, &allocInfo, ALLOCATOR, &memory),
      std::format("failed to allocate buffer memory '{}_memory'", name));

  NAME_OBJECT(device, VK_OBJECT_TYPE_BUFFER, buffer,
              std::format("{}_buffer", name));
  NAME_OBJECT(device, VK_OBJECT_TYPE_DEVICE_MEMORY, memory,
              std::format("{}_memory", name));
}

Vulking::Buffer::~Buffer() {
  assert(buffer != VK_NULL_HANDLE);
  vkDestroyBuffer(device, buffer, ALLOCATOR);
  buffer = VK_NULL_HANDLE;

  assert(memory != VK_NULL_HANDLE);
  vkFreeMemory(device, memory, ALLOCATOR);
  memory = VK_NULL_HANDLE;
}

void Vulking::Buffer::map() { mapTo(&pData); }

void Vulking::Buffer::mapTo(void **mapped) {
  CHK(vkMapMemory(device, memory, 0, size, 0, mapped),
      "failed to map buffer memory");
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
  vkUnmapMemory(device, memory);
  pData = nullptr;
}
