#include "CommandPool.hpp"
Vulking::CommandPool::CommandPool(const PhysicalDevice &physicalDevice,
                                  const Device &device)
    : physicalDevice(physicalDevice), device(device) {}

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
