#include "StagingBuffer.hpp"

Vulking::StagingBuffer::StagingBuffer(const PhysicalDevice &physicalDevice,
                                      const Device &device, void *data,
                                      VkDeviceSize size, const char *name)
    : device(device) {
  VulkingUtil::createBuffer(physicalDevice, device, size,
                            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                            buffer, memory, name);

  CHK(vkMapMemory(device, memory, 0, size, 0, &dst),
      "failed to map buffer memory");
  memcpy(dst, data, static_cast<size_t>(size));
  vkUnmapMemory(device, memory);
}
Vulking::StagingBuffer::~StagingBuffer() {
  vkDestroyBuffer(device, buffer, allocator);
  vkFreeMemory(device, memory, allocator);
}

Vulking::StagingBuffer::operator VkBuffer() const { return buffer; }
