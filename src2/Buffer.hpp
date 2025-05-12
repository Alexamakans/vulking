#pragma once

#include "Common.hpp"
#include "Device.hpp"
#include <vulkan/vulkan_core.h>

namespace Vulking {
class Buffer {
public:
  Buffer(Device device, void *src, VkDeviceSize size, VkBufferUsageFlags usage,
         VkMemoryPropertyFlags properties, const char *name = "unnamed") {
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

private:
  VkBuffer buffer;
  VkDeviceMemory memory;
  void *data;
};
} // namespace Vulking
