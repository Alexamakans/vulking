#include "CommandPool.hpp"

Vulking::CommandPool::CommandPool(const PhysicalDevice &physicalDevice,
                                  const Device &device)
    : physicalDevice(physicalDevice), device(device),
      commandPool(createCommandPool()) {}

void Vulking::CommandPool::allocateBuffer(VkCommandBufferLevel level,
                                          size_t count,
                                          VkCommandBuffer *buffers) const {
  VkCommandBufferAllocateInfo info{};
  info.sType = STYPE(COMMAND_BUFFER_ALLOCATE_INFO);
  info.commandPool = commandPool;
  info.level = level;
  info.commandBufferCount = static_cast<uint32_t>(count);

  CHK(vkAllocateCommandBuffers(device, &info, buffers),
      "failed to allocate command buffers");
}

VkCommandPool Vulking::CommandPool::createCommandPool() {
  VkCommandPoolCreateInfo info{};
  info.sType = STYPE(COMMAND_POOL_CREATE_INFO);
  info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  info.queueFamilyIndex =
      physicalDevice.queueFamilyIndices.graphicsFamily.value();

  VkCommandPool _commandPool;
  CHK(vkCreateCommandPool(device, &info, allocator, &_commandPool),
      "failed to create command pool");

  return _commandPool;
}
