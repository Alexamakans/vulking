#pragma once

#include "../common.hpp"

#include "../resources/StagingBuffer.hpp"
#include "../wrappers/CommandPool.hpp"
#include "../wrappers/Device.hpp"
#include "../wrappers/PhysicalDevice.hpp"
#include <vulkan/vulkan_core.h>

namespace Vulking {
class Model {
public:
  Model(const PhysicalDevice &physicalDevice, const Device &device,
        const CommandPool &commandPool, const Queue &queue,
        const std::string &path);

  void release() const;
  void bindBuffers(VkCommandBuffer commandBuffer);
  uint32_t getNumIndices() const;

private:
  const PhysicalDevice &physicalDevice;
  const Device &device;

  VkBuffer vertexBuffer;
  VkDeviceMemory vertexBufferMemory;
  uint32_t numVertices;

  VkBuffer indexBuffer;
  VkDeviceMemory indexBufferMemory;
  uint32_t numIndices;

  template <typename T>
  void createBuffer(const PhysicalDevice &physicalDevice, const Device &device,
                    const CommandPool &commandPool, const Queue &queue,
                    std::vector<T> elements, VkBufferUsageFlags usage,
                    VkBuffer &buffer, VkDeviceMemory &memory,
                    const char *name = "unnamed") {

    VkDeviceSize size = sizeof(elements[0]) * elements.size();
    StagingBuffer stagingBuffer(physicalDevice, device, elements.data(), size,
                                "model_staging_buffer");
    VulkingUtil::createBuffer(physicalDevice, device, size, usage,
                              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer,
                              memory, name);
    VulkingUtil::copyBuffer(device, commandPool, queue, stagingBuffer, buffer,
                            size);
  }
};
} // namespace Vulking
