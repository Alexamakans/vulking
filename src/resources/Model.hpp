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

  ~Model();

private:
  const PhysicalDevice &physicalDevice;
  const Device &device;

  VkBuffer vertexBuffer;
  VkDeviceMemory vertexBufferMemory;

  VkBuffer indexBuffer;
  VkDeviceMemory indexBufferMemory;

  template <typename T>
  void createBuffer(const PhysicalDevice &physicalDevice, const Device &device,
                    const CommandPool &commandPool, const Queue &queue,
                    std::vector<T> elements, VkBufferUsageFlags usage,
                    VkBuffer &buffer, VkDeviceMemory &memory);
};

template <typename T>
inline void
Model::createBuffer(const PhysicalDevice &physicalDevice, const Device &device,
                    const CommandPool &commandPool, const Queue &queue,
                    std::vector<T> elements, VkBufferUsageFlags usage,
                    VkBuffer &buffer, VkDeviceMemory &memory) {
  VkDeviceSize size = sizeof(elements[0]) * elements.size();
  StagingBuffer stagingBuffer(physicalDevice, device, elements.data());
  VulkingUtil::createBuffer(physicalDevice, device, size, usage,
                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer,
                            memory);
  VulkingUtil::copyBuffer(device, commandPool, queue, stagingBuffer, buffer,
                          size);
}

} // namespace Vulking
