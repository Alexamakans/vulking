#pragma once

#include "../common.hpp"

#include "../helpers/VulkingUtil.hpp"
#include "../wrappers/Device.hpp"
#include "../wrappers/PhysicalDevice.hpp"
#include <vulkan/vulkan_core.h>

namespace Vulking {
template <typename T> class UniformBuffer {
public:
  UniformBuffer(const PhysicalDevice &physicalDevice, const Device &device)
      : physicalDevice(physicalDevice), device(device) {
    size = sizeof(T);
    VulkingUtil::createBuffer(physicalDevice, device, sizeof(T),
                              VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                  VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                              buffer, memory);
    CHK(vkMapMemory(device, memory, 0, size, 0, &data),
        "failed to map uniform buffer memory");
  };

  ~UniformBuffer() {
    vkDestroyBuffer(device, buffer, allocator);
    vkFreeMemory(device, memory, allocator);
    data = nullptr;
  }

  void set(T *t) const { memcpy(data, t, sizeof(T)); };
  T *get() { return reinterpret_cast<T *>(data); }

private:
  const PhysicalDevice &physicalDevice;
  const Device &device;

  VkDeviceSize size;
  VkBuffer buffer;
  VkDeviceMemory memory;
  void *data;
};
} // namespace Vulking
